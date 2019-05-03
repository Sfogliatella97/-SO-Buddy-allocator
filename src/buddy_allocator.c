#include "buddy_allocator.h"
#include "binary_tree.h"

#include "debug_utilities.h"

#define FREE_FLAG (1)
#define OCCUPIED_FLAG (!(FREE_FLAG))

#define min(a,b) (((a) < (b))? (a) : (b))

#define buddy_allocator_b_tree_address(allocator) ((allocator) + sizeof(buddy_allocator))


    //NEEDS TO BE MORE CLEAR
/*
    Computes the amount of levels for which a chunk of <buffer_size> corresponding to
    the last level is such that (sizeof(chunk) >> 1) < min_bucket_size
*/

static unsigned levels_needed(unsigned buffer_size, unsigned min_bucket_size)
{

    dbug_n("levels_needed");

    unsigned levels;
    for(levels = 0; ( buffer_size >> (levels + 1) ) >= min_bucket_size ; levels++);

    dbug_formatted_print("\tbuffer_size: %u\n", buffer_size);
    dbug_formatted_print("\tmin_bucket_size: %u\n", min_bucket_size);
    dbug_formatted_print("\tlevels_needed: %u\n", levels);

    dbug_e("levels_needed");

    return levels;
}

/*
    Given the size of a buffer, computes the amount of levels we can fit in there (rounded up)
*/

static unsigned fitting_levels(unsigned size)
{

    dbug_n("fitting_levels");

    dbug_formatted_print("\tSize: %u", size);

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
    
    dbug_formatted_print("\tfitting_levels: %u\n", levels);

    dbug_e("fitting_levels");
    
    return levels;
}

/*
    Given a buddy allocator and an amount of memory to allocate, computes the minimum level such that
    mem_req can still be positioned in a chunk of that level.
*/

static unsigned appropriate_level(buddy_allocator* allocator, unsigned mem_req)
{

    dbug_n("appropriate_level");

    unsigned level = 0;
    unsigned chunk_size = allocator->buffer_size;
    while( ((level + 1) <= allocator->levels ) && 
           ((chunk_size >>= 1) >= mem_req)         )
        level++;

    dbug_formatted_print("\tallocator->buffer_size: %u\n", allocator->buffer_size);
    dbug_formatted_print("\tallocator->levels: %u\n", allocator->levels);
    dbug_formatted_print("\tmem_req: %u\n", mem_req);
    dbug_formatted_print("\tappropriate level: %u\n", level);

    dbug_e("appropriate_level");

    return level;
}

/*
    Given an index in the binary tree, returns the corresponding address in the buffer.
    This could be a lot easier with the assumption that buffer_size = 2^levels.
    This needs to be addressed in the next version.
*/

void* index2address(buddy_allocator* allocator, unsigned index)
{

    dbug_n("index2address");

    /*
        Since we could have <allocator->buffer> which is not a power of 2 * <min_bucket_size>,
        we can't determine the address directly from the index. So we climb up the tree,
        storing into <array_which_child> our path, and then we come back with the address.
    */
    char* chunk = allocator->buffer;
    unsigned chunk_size = allocator->buffer_size >> 1;

    int array_which_child[allocator->levels];

    dbug_formatted_print("\tallocator->buffer: %p\n", allocator->buffer);
    dbug_formatted_print("\tindex: %u\n", index);

    unsigned ancestor = index;
    unsigned count;
    for(count = 0; ancestor != 0; count++, ancestor = parent_index(ancestor))
        array_which_child[count] = (right_child_index(parent_index(ancestor)) == ancestor);
    
    dbug_formatted_print("\tallocator->levels: %u\n", allocator->levels);

    for(; ancestor != index; count--, chunk_size >>= 1)
    {
        if(array_which_child[count - 1]) 
        {
            chunk += chunk_size;
            ancestor = right_child_index(ancestor);
        }
        else
            ancestor = left_child_index(ancestor);
    }

    dbug_formatted_print("\taddress: %p\n", chunk);
    dbug_formatted_print("\taddress - buffer: %d\n", (chunk - allocator->buffer ));

    dbug_e("index2address");

    return chunk;
}

/*
    Given a valid <address> in <allocator->buffer>, returns its corresponding
    index in <b_uddy_allocator_b_tree_address(allocator)>; if <address> is not 
    valid, returns <allocator->b_tree_length> 
*/

unsigned address2index(buddy_allocator* allocator, void* address)
{

    dbug_n("address2index");

    if((address < ((void*)allocator->buffer)) || 
            (address >= ((void*)(allocator->buffer + allocator->buffer_size)))
            )
    {
        dbug_e("address2index");
        return allocator->b_tree_length;
    }

    b_tree* tree = buddy_allocator_b_tree_address(allocator);
    unsigned offset = address - ((void*) allocator->buffer);

    dbug_formatted_print("\tallocator->buffer: %p\n", allocator->buffer);
    dbug_formatted_print("\taddress: %p\n", address);
    dbug_formatted_print("\toffset: %d\n", offset);

    unsigned index = 0;
    unsigned current_offset = 0;
    unsigned current_size = allocator->buffer_size;

#if DEBUG_MODE
unsigned u = 0;
#endif

    for(; (index < allocator->b_tree_length) && (b_tree_get(tree, index) == OCCUPIED_FLAG);)
    {
        current_size >>= 1;
        
        if((offset == current_offset) &&  (b_tree_get(tree, left_child_index(index)) == FREE_FLAG))
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

    dbug_formatted_print("\n\titeration: %u\n\n", u++);
    dbug_formatted_print("\tindex: %u\n", index);
    dbug_formatted_print("\tcurrent_offset: %u\n", current_offset);
    dbug_formatted_print("\tcurrent_size: %u\n", current_size);

    }

    dbug_e("address2index");

    return allocator->b_tree_length;
}

unsigned buddy_allocator_memrequired(unsigned buffer_size, unsigned min_bucket_size)
{

    dbug_n("buddy_allocator_memrequired");

    dbug_formatted_print("\tbuffer_size: %u\n", buffer_size);
    dbug_formatted_print("\tmin_bucket_size: %u\n", min_bucket_size);
    dbug_formatted_print("\tmemreq: %u\n", ((unsigned) sizeof(buddy_allocator) ) + 
            b_tree_complete_memrequired( levels_needed(buffer_size, min_bucket_size) ) );

    dbug_e("buddy_allocator_memrequired");

    return sizeof(buddy_allocator) + 
            b_tree_complete_memrequired(levels_needed(buffer_size, min_bucket_size));
}

int buddy_allocator_init(void* working_memory, unsigned working_memory_size,
                         char* buffer, unsigned buffer_size,
                         unsigned min_bucket_size)
{
    int error_code = 0;

    dbug_n("buddy_allocator_init");

    dbug_formatted_print("\tworking_memory: %p\n", working_memory);
    dbug_formatted_print("\tworking_memory_size: %u\n", working_memory_size);
    dbug_formatted_print("\tbuffer: %p\n", buffer);
    dbug_formatted_print("\tbuffer_size: %u\n", buffer_size);
    dbug_formatted_print("\tmin_bucket_size: %u\n", min_bucket_size);
    /*
        If there's no room for a buddy_allocator struct in <working_memory>,
        we return an error code and abort.
    */

    if(buffer_size < (sizeof(buddy_allocator) + b_tree_complete_memrequired(1)))
    {

    dbug_e("buddy_allocator_init");

        return -1;
    }

    /*
        We check if we can fit in <working_memory> a tree big enough to manage
        efficiently the <buffer>, given the minimum workload <min_bucket_size>
    */

    if(buffer_size < buddy_allocator_memrequired(buffer_size, min_bucket_size))
        error_code += 1;

    /*
        Initializing a buddy_allocator in <working_memory>
    */

    buddy_allocator* allocator = (buddy_allocator*) working_memory;
    
        allocator->buffer = buffer;
        allocator->buffer_size = buffer_size;
        allocator->b_tree_length = b_tree_fitting_length(working_memory_size - sizeof(buddy_allocator));
        allocator->levels = fitting_levels(working_memory_size - sizeof(buddy_allocator));
        allocator->greatest_free_index = 0;
        allocator->greatest_free_index_size = buffer_size;

    /*
        Initializing the binary tree
    */

    dbug_formatted_print("\tb_tree_address: %p\n", buddy_allocator_b_tree_address(allocator));
    dbug_formatted_print("\tb_tree_length: %u\n", allocator->b_tree_length );

    b_tree_init(buddy_allocator_b_tree_address(allocator), allocator->b_tree_length);

    /*
        The whole buffer is free at the beginning
    */

    unsigned u;
    for(u = 0; u < allocator->b_tree_length; u++)
        b_tree_put(buddy_allocator_b_tree_address(allocator), u, FREE_FLAG);

    dbug_formatted_print("\tLevels_needed: %u\n", levels_needed(buffer_size, min_bucket_size));
    dbug_formatted_print("\tFitting levels: %u\n", allocator->levels);

    dbug_e("buddy_allocator_init");

    return error_code;
}

/*
    It works, but needs to be rewritten. It should be O(levels) but is actually O(2^levels)
*/
void* buddy_allocator_malloc(buddy_allocator* allocator, unsigned size)
{
    
    dbug_n("buddy_allocator_malloc");

    unsigned level = appropriate_level(allocator, size);
    b_tree* tree = buddy_allocator_b_tree_address(allocator);

    unsigned lim = min( allocator->b_tree_length - 1, max_index_on_level(level) );
    
    dbug_formatted_print("\tsize: %u\n", size );
    dbug_formatted_print("\tlimit_index: %u\n", lim );

    unsigned u;
    for(u = 0; u <= lim; u++)
    {
        if(b_tree_get(tree, u) == FREE_FLAG)
        {
            b_tree_put(tree, u, OCCUPIED_FLAG);

            /*
                Getting to the appropriate level
                (We chose arbitrarily to fill always the left sub-tree first. This doesn't change
                anything, since every node will be considered by the outer iteration)
            */
            for(;left_child_index(u) <= lim; u = left_child_index(u))
                b_tree_put(tree, u, OCCUPIED_FLAG);
            
            dbug_formatted_print("\tIndex found: %u\n", u );
            dbug_formatted_print("\tCorresponding address: %p\n", index2address(allocator, u) );

            dbug_e("buddy_allocator_malloc");
            
            return index2address(allocator, u);
        }
    }

    dbug_print("Not enough memory");

    dbug_e("buddy_allocator_malloc");

    return NOT_ENOUGH_MEMORY(allocator);
}

//Will probably need to be rewritten as well
void buddy_allocator_free(buddy_allocator* allocator, void* address)
{

    dbug_n("buddy_allocator_free");

    dbug_formatted_print("\taddress: %p\n", address );

    b_tree* tree = buddy_allocator_b_tree_address(allocator);

    unsigned index = address2index(allocator, address);

    dbug_formatted_print("\tcorresponding index: %u\n", index );

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

    dbug_e("buddy_allocator_free");

}