#include "buddy_allocator.h"
#include "binary_tree.h"
#include "stack.h"

#include "debug_utilities.h"

#define FREE_FLAG (1)
#define OCCUPIED_FLAG (!(FREE_FLAG))

#define min(a,b) (((a) < (b))? (a) : (b))

/*
    Returns the address of the binary tree. (It's right after the buddy_allocator struct)
*/

#define buddy_allocator_b_tree_address(allocator) ( ((char*)(allocator)) + sizeof(buddy_allocator) )

/*
    Returns the lowest level whose memory is still bigger than <size> (Used by malloc)
*/

#define best_fit_level(allocator, size) min(allocator->levels, required_levels(allocator->buffer_size, size))


/*
    Computes the amount of levels for which a chunk of <buffer_size> corresponding to
    the last level is such that (sizeof(chunk) >> 1) < min_bucket_size (i.e. a lower level
    would represents chunks which are smaller than min_bucket_size)
*/

static unsigned required_levels(unsigned buffer_size, unsigned min_bucket_size)
{

    unsigned levels;
    for(levels = 0; ( buffer_size >> (levels + 1) ) >= min_bucket_size ; levels++);

    return levels;
}

/*
    Given the size of a buffer, computes the amount of levels we can fit in there (rounded up)
*/

static unsigned fitting_levels(unsigned size)
{

    unsigned levels;
    
    for(levels = 0; ; levels++)
    {
        if(b_tree_complete_memrequired(levels + 1 ) > size)
            break;
        if(b_tree_complete_memrequired(levels + 1) == size)
        {
            levels++;
            break;
        }
    }
    
    return levels;
}


/*
    Given an index in the binary tree, returns the corresponding address in the buffer.
    This could be a lot easier with the assumption that buffer_size = 2^levels.
    This needs to be addressed in the next version.
*/

void* index2address(buddy_allocator* allocator, unsigned index)
{
    dbug_formatted_print("Index is: %u\n", index);
    /*
        Since we could have <allocator->buffer> which is not a power of 2 * <min_bucket_size>,
        we can't determine the address directly from the index. So we climb up the tree,
        storing into <array_which_child> our path, and then we come back with the address.
    */
    char* chunk = allocator->buffer;
    unsigned chunk_size = allocator->buffer_size >> 1;

    int array_which_child[allocator->levels];

    unsigned count;
    for(count = 0; index != 0; count++, index = parent_index(index))
        array_which_child[count] = (right_child_index(parent_index(index)) == index);


    for(; count != 0; count--, chunk_size >>= 1)
        if(array_which_child[count - 1]) 
            chunk += chunk_size;

    dbug_formatted_print("Offset is: %ld\n\n", offset(allocator->buffer, chunk));

    return chunk;
}

/*
    Given a valid <address> in <allocator->buffer>, returns its corresponding
    index in <b_uddy_allocator_b_tree_address(allocator)>; if <address> is not 
    valid, returns <allocator->b_tree_length> 
*/

unsigned address2index(buddy_allocator* allocator, void* address)
{

    if(( ((char*)address) < allocator->buffer) || 
            (
                ((char*)address) >= (allocator->buffer + allocator->buffer_size)
            )
    )
        return allocator->b_tree_length; 

    b_tree* tree = buddy_allocator_b_tree_address(allocator);
    long int offset = ((char*)address) - allocator->buffer;

    unsigned index = 0;
    long int current_offset = 0;
    long int current_size = allocator->buffer_size;

    for(; (index < allocator->b_tree_length) && (b_tree_get(tree, index) == OCCUPIED_FLAG);)
    {
        current_size >>= 1;
        
        if((offset == current_offset) && 
                (
                    (left_child_index(index) >= allocator->b_tree_length) || 
                    (b_tree_get(tree, left_child_index(index)) == FREE_FLAG)
                )
        )
            return index;

        /*
            The address is in the left subtree
        */

        if(offset < (current_offset + current_size))
        {
            index = left_child_index(index);
        }
        else
        {
            index = right_child_index(index);
            current_offset += current_size;
        }
    }

    return allocator->b_tree_length;
}

unsigned buddy_allocator_memrequired(unsigned buffer_size, unsigned min_bucket_size)
{
    return sizeof(buddy_allocator) + 
            b_tree_complete_memrequired(required_levels(buffer_size, min_bucket_size));
}

int buddy_allocator_init(void* working_memory, unsigned working_memory_size,
                         char* buffer, unsigned buffer_size,
                         unsigned min_bucket_size)
{
    int error_code = 0;

    /*
        If there's no room for a buddy_allocator struct in <working_memory>,
        we return an error code and abort.
    */

    if(working_memory_size < (sizeof(buddy_allocator) + b_tree_complete_memrequired(1)))
        return -1;

    /*
        We check if we can fit in <working_memory> a tree big enough to manage
        efficiently the <buffer>, given the minimum workload <min_bucket_size>
    */

    if(working_memory_size < buddy_allocator_memrequired(buffer_size, min_bucket_size))
        error_code += 1;

    /*
        Initializing a buddy_allocator in <working_memory>
    */

    buddy_allocator* allocator = (buddy_allocator*) working_memory;
    
        allocator->buffer = buffer;
        allocator->buffer_size = buffer_size;
        allocator->b_tree_length = b_tree_fitting_length(working_memory_size - sizeof(buddy_allocator));
        allocator->levels = fitting_levels(working_memory_size - sizeof(buddy_allocator));

    /*
        Initializing the binary tree
    */

    b_tree_init(buddy_allocator_b_tree_address(allocator), allocator->b_tree_length);

    /*
        The whole buffer is free at the beginning
    */

    unsigned u;
    for(u = 0; u < allocator->b_tree_length; u++)
        b_tree_put(buddy_allocator_b_tree_address(allocator), u, FREE_FLAG);

    for(u = 0; u < allocator->b_tree_length; u++)
        printf("%d", b_tree_get(buddy_allocator_b_tree_address(allocator), u));
    printf("\ninit\n");

    return error_code;
}

/*
    It works, but needs to be rewritten. It should be O(levels) but is actually O(2^levels)
*/
void* buddy_allocator_malloc(buddy_allocator* allocator, unsigned size)
{
    unsigned level = best_fit_level(allocator, size);
    b_tree* tree = buddy_allocator_b_tree_address(allocator);

    unsigned lim = min( allocator->b_tree_length - 1, max_index_on_level(level) );

    dbug_formatted_print("memrequired: %u\n", size);

    char mem[stack_mem_required(allocator->b_tree_length) >> 1]; //It probably needs way less than
    // <allocator->b_tree_length>. needs to be checked again

    stack* stack = stack_init(mem); 

    stack_push(stack, 0);
    unsigned u, left_child, right_child;
    while(!stack_is_empty(stack))
    {
        u = stack_pop(stack);
        
        if(b_tree_get(tree, u) == FREE_FLAG) 
        {
            dbug_formatted_print("FLAG is: %d\n", b_tree_get(tree, u));
            b_tree_put(tree, u, OCCUPIED_FLAG);
            /*
                Getting to the required level
                (We chose arbitrarily to fill always the left sub-tree first. This doesn't change
                anything, since every node will be considered by the outer iteration)
            */
            for(;left_child_index(u) <= lim; u = left_child_index(u))
            {
                dbug_formatted_print("FLAG is: %d", b_tree_get(tree, u));
                b_tree_put(tree, u, OCCUPIED_FLAG);
            }
            

            if(u == 383)
                dbug_formatted_print("\nParent was: %d\n", b_tree_get(tree, u));

            b_tree_put(tree, u, OCCUPIED_FLAG);

            if(u == 383)
                dbug_formatted_print("383 is %d\n", b_tree_get(tree, u));

            return index2address(allocator, u);
        }
        else
        {
            left_child = left_child_index(u);
            right_child = right_child_index(u);

            if( (left_child > lim) || 
                    ( 
                        (b_tree_get(tree, left_child) == FREE_FLAG) &&
                        (b_tree_get(tree, right_child) == FREE_FLAG)
                    )
              )
                continue;
            else
            {
                stack_push(stack, left_child);
                stack_push(stack, right_child);
            }
        }
    }

    dbug_formatted_print("index (not valid): %u\n\n", u);

    return NOT_ENOUGH_MEMORY(allocator);
}

//Will probably need to be rewritten as well
void buddy_allocator_free(buddy_allocator* allocator, void* address)
{
    b_tree* tree = buddy_allocator_b_tree_address(allocator);

    unsigned index = address2index(allocator, address);

    /*
        If address2index returns an index equal to the length of the tree,
        then the address was not valid and we do nothing
    */
    
    if(index == allocator->b_tree_length)
        return;

    /*
        We could do some clean-up for security reasons here, if needed.
        to be included as an option in the next version
    */

        //TODO

    /*
        If both an address and its buddy are now free, then we bring them back together
        and do the same with their parents
    */
    
    b_tree_put(tree, index, FREE_FLAG);

    for(;(index != 0) && 
            (
            (sibling_index(index) >= allocator->b_tree_length) ||
            (b_tree_get(tree, sibling_index(index)) == FREE_FLAG)
            ); 
            index = parent_index(index)
                )
        b_tree_put(tree, parent_index(index), FREE_FLAG);
}