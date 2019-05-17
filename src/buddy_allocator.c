#include "buddy_allocator.h"
#include "binary_tree.h"
#include "stack.h"

#include "debug_utilities.h"

#define FREE_FLAG (1)
#define OCCUPIED_FLAG (!(FREE_FLAG))

#define NO_AVAILABLE_INDEX(allocator) ((allocator)->b_tree_length)

#define min(a,b) (((a) < (b))? (a) : (b))

/*
    Returns the address of the binary tree. (It's right after the struct buddy_allocator)
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
    
        for(levels = 0; b_tree_complete_memrequired(levels + 1 ) <= size ; levels++)
                ;
    
        return levels;
}


/*
    Given an index in the binary tree, returns the corresponding address in the buffer.
    This could be a lot easier with the assumption that buffer_size = 2^levels.
    This needs to be addressed in the next version.
*/

static inline void* index2address(buddy_allocator* allocator, unsigned index)
{
        char*    chunk;
        unsigned chunk_size;
        unsigned count;
        /*
            Since we could have <allocator->buffer> which is not a power of 2 * <min_bucket_size>,
            we can't determine the address directly from the index. So we climb up the tree,
            storing into <array_which_child> our path, and then we come back with the address.
        */
        chunk      = allocator->buffer;
        chunk_size = allocator->buffer_size >> 1;

        int array_which_child[allocator->levels];

        for(count = 0; index != 0; count++, index = parent_index(index))
                array_which_child[count] = (right_child_index(parent_index(index)) == index);


        for(; count != 0; count--, chunk_size >>= 1)
                if(array_which_child[count - 1]) 
                        chunk += chunk_size;

        return chunk;
}

/*
    Given a valid <address> in <allocator->buffer>, returns its corresponding
    index in <b_uddy_allocator_b_tree_address(allocator)>; if <address> is not 
    valid, returns <allocator->b_tree_length> 
*/

static inline unsigned address2index(buddy_allocator* allocator, void* address)
{
        unsigned index;
        long int current_offset;
        long int current_size;
        long int offset;
        b_tree*  tree;

        if(( ((char*)address) < allocator->buffer) || 
            (
                ((char*)address) >= (allocator->buffer + allocator->buffer_size)
            )
        )
                return allocator->b_tree_length;

        tree           = buddy_allocator_b_tree_address(allocator);
        offset         = ((char*)address) - allocator->buffer;
        index          = 0;
        current_offset = 0;
        current_size   = allocator->buffer_size;

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

                if(offset < (current_offset + current_size))
                {
                        /*
                            The address is in the left subtree
                        */
                        index = left_child_index(index);
                }
            
                else
                {
                        /*
                            It's in the right one
                        */
                        index           = right_child_index(index);
                        current_offset += current_size;
                }
        }

        return allocator->b_tree_length;
}

/*
    The greatest available index within 0 and lim.
    If there is no available index, returns NO_AVAILABLE_INDEX(allocator)
*/

static inline unsigned greatest_available_index(buddy_allocator* allocator, unsigned lim)
{
        unsigned u;
        unsigned left_child;
        unsigned right_child;
        b_tree*  tree;
        stack*   stack;

        tree  = buddy_allocator_b_tree_address(allocator);
        char mem[stack_mem_required(allocator->b_tree_length) >> 1];
        stack = stack_init(mem); 
        stack_push(stack, 0);

        /*
            The tree is such that if a node is <OCCUPIED_FLAG> there can be 2 possibilities:
            1) Both its children are <FREE_FLAG>: This means that all of the corresponding 
               memory chunk is allocated
            2) At least one of its children are <OCCUPIED_FLAG>: This means that a part of 
               the corresponding chunk is allocated. So, if we actually need a smaller node, 
               we should also examine its descendants.
        
            Thus, we need to be careful with "free" nodes. In fact, even if a node is <FREE_FLAG>, 
            it could actually be a descendant of a <OCCUPIED_FLAG>, since all of an occupied node's 
            descendants are <FREE_FLAG>. To prevent these nodes from being examined we, put the root in
            a stack and for every node in the stack:
            1) It is <FREE_FLAG>: We allocate it
            2) It is <OCCUPIED_FLAG> and at least one of its children is <OCCUPIED_FLAG>: we put both 
               its children on the stack
            3) IT is <OCCUPIED_FLAG> and both its children are <FREE_FLAG>: we do nothing
        */

        while(!stack_is_empty(stack))
        {
                u = stack_pop(stack);
        
                if(b_tree_get(tree, u) == FREE_FLAG) 
                        return u;

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

        return NO_AVAILABLE_INDEX(allocator);
}


unsigned buddy_allocator_memrequired(unsigned buffer_size, unsigned min_bucket_size)
{
        return sizeof(buddy_allocator) + 
                b_tree_complete_memrequired(required_levels(buffer_size, min_bucket_size));
}

int buddy_allocator_init(buddy_allocator** allocator, void* working_memory, 
                         unsigned working_memory_size, char* buffer, unsigned buffer_size,
                         unsigned min_bucket_size)
{
        int              error_code;
        unsigned         u;
        buddy_allocator* alloc;

        error_code = 0;

        /*
            If there's no room for a buddy_allocator struct in <working_memory>,
            we return an error code and abort.
        */

        if(working_memory_size < buddy_allocator_memrequired(buffer_size, buffer_size))
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

        *allocator = (buddy_allocator*) working_memory;
        alloc      = *allocator;
    
        alloc->buffer        = buffer;
        alloc->buffer_size   = buffer_size;
        alloc->b_tree_length = b_tree_fitting_length(working_memory_size - sizeof(buddy_allocator));
        alloc->levels        = fitting_levels(working_memory_size - sizeof(buddy_allocator));

        /*
            Initializing the binary tree
        */

        b_tree_init(buddy_allocator_b_tree_address(alloc), alloc->b_tree_length);

        /*
            The whole buffer is free at the beginning
        */

        for(u = 0; u < alloc->b_tree_length; u++)
                b_tree_put(buddy_allocator_b_tree_address(alloc), u, FREE_FLAG);

        return error_code;
}

/*
    It works, but needs to be rewritten. It should be O(levels) but is actually O(2^levels)
*/
void* buddy_allocator_malloc(buddy_allocator* allocator, unsigned size)
{
        unsigned level;
        unsigned u;
        unsigned left_child;
        unsigned right_child;
        unsigned last_available_index;
        unsigned lim;
        b_tree*  tree;
        stack*   stack;

        tree  = buddy_allocator_b_tree_address(allocator);
        level = best_fit_level(allocator, size);
        lim   = min( allocator->b_tree_length - 1, max_index_on_level(level) );

        /*
            The reasoning is similar to greatest_available_index(allocator, lim), but this 
            time we search for the last available index. In fact, if we used the greatest, 
            there would be external fragmentation: for example, if we had a buffer of 2048 bytes 
            and we called buddy_allocator_malloc(allocator, 1) twice, 
            buddy_allocator(allocator, 1024) would fail, even tough there should be enough memory.
        */

        char mem[stack_mem_required( (allocator->b_tree_length) >> 1)];
        stack = stack_init(mem); 
        stack_push(stack, 0);
        last_available_index = NO_AVAILABLE_INDEX(allocator);

        while(!stack_is_empty(stack))
        {
                u = stack_pop(stack);
        
                if(b_tree_get(tree, u) == FREE_FLAG) 
                        last_available_index = u;

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

        if(last_available_index == NO_AVAILABLE_INDEX(allocator))
                return NOT_ENOUGH_MEMORY(allocator);
    
        else
        {
                u = last_available_index;
                b_tree_put(tree, u, OCCUPIED_FLAG);
                /*
                    Getting to the required level
                    (We chose arbitrarily to fill always the left sub-tree first. This doesn't change
                    anything, since every node will be considered by the outer iteration)
                */
                for(;left_child_index(u) <= lim; u = left_child_index(u))
                        b_tree_put(tree, u, OCCUPIED_FLAG);
            

                b_tree_put(tree, u, OCCUPIED_FLAG);


                return index2address(allocator, u);
        }
}

//Will probably need to be rewritten as well
void buddy_allocator_free(buddy_allocator* allocator, void* address)
{
        b_tree*  tree;
        unsigned index;

        tree  = buddy_allocator_b_tree_address(allocator);
        index = address2index(allocator, address);
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

        for(;(
                (index != 0) && 
                (
                    (sibling_index(index) >= allocator->b_tree_length) ||
                    (b_tree_get(tree, sibling_index(index)) == FREE_FLAG)
                )
            ); index = parent_index(index)  )

                b_tree_put(tree, parent_index(index), FREE_FLAG);
}

unsigned buddy_allocator_available_mem(buddy_allocator* allocator)
{
        unsigned u;
        unsigned count;
        unsigned size;

        u = greatest_available_index(allocator, allocator->b_tree_length - 1);


        if(u == NO_AVAILABLE_INDEX(allocator)) 
                return 0;

        size = allocator->buffer_size;

        for(count = 0; u > ( (1 << (count + 1)) - 1 ); count++, size >>= 1 )
                ;

        return size;
}
