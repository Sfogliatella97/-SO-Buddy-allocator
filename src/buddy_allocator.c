#ifdef DEBUG_MODE
#undef DEBUG_MODE
#endif

#define DEBUG_MODE 1

#include "buddy_allocator.h"
#include "binary_tree.h"

#if DEBUG_MODE
#include <stdio.h>
#endif 

#define FREE_FLAG (1)
#define OCCUPIED_FLAG (!(FREE_FLAG))

#define min(a,b) (((a) < (b))? (a) : (b))

#define buddy_allocator_b_tree_nodes(working_memory_size) \
                (b_tree_memrequired((working_memory_size) - sizeof(buddy_allocator)))

    //NEEDS TO BE MORE CLEAR
/*
    Computes the amount of levels for which a chunk of <buffer_size> corresponding to
    the last level is such that (sizeof(chunk) >> 1) < min_bucket_size
*/

static unsigned levels_needed(unsigned buffer_size, unsigned min_bucket_size)
{
    unsigned levels = 0;
    while((buffer_size >> (levels+1)) > (min_bucket_size))
        levels++;

#if DEBUG_MODE
printf("----------------------------------------------------------------------------------\n\
Function:  levels_needed:\n\tbuffer_size: %u\n\tmin_bucket_size: %u\n\tlevels_needed: %u\n\
----------------------------------------------------------------------------------\n"
, buffer_size, min_bucket_size, levels);
#endif

    return levels;
}

/*
    Given the size of a buffer, computes the amount of levels we can fit in there (rounded up)
*/

static unsigned fitting_levels(unsigned working_memory_size)
{
    unsigned count= 1;
    while(((1 << count)-1) < working_memory_size)
        count++;
    
#if DEBUG_MODE
printf("----------------------------------------------------------------------------------\n\
Function:  fitting_levels:\n\tworking_memory_size: %u\n\tfitting_levels: %u\n\
----------------------------------------------------------------------------------\n"
, working_memory_size, count);
#endif
    
    return count;
}

/*
    Given a buddy allocator and an amount of memory to allocate, computes the minimum level such that
    mem_req can still be positioned in a chunk of that level.
*/

static unsigned appropriate_level(buddy_allocator* allocator, unsigned mem_req)
{
    unsigned level = 0;
    unsigned chunk_size = allocator->buffer_size;
    while( ((level + 1) <= allocator->levels ) && ((chunk_size >>= 1) > mem_req))
        level++;

#if DEBUG_MODE
printf("----------------------------------------------------------------------------------\n\
Function:  appropriate_level:\n\tallocator->buffer_size: %u\n\tallocator->levels: %u\n\t\
mem_req: %u\n----------------------------------------------------------------------------------\n"
, allocator->buffer_size, allocator->levels, mem_req);
#endif

    return level;
}

/*
    Given an index in the binary tree, returns the corresponding address in the buffer.
    This could be a lot easier with the assumption that buffer_size = 2^levels.
    This needs to be addressed in the next version.
*/

void* index2address(buddy_allocator* allocator, unsigned index)
{
    /*
        Since we could have <allocator->buffer> which is not a power of 2 * <min_bucket_size>,
        we can't determine the address directly from the index. So we climb up the tree,
        storing into <array_which_child> our path, and then we come back with the address.
    */
    void* chunk = allocator->buffer;
    unsigned chunk_size = allocator->buffer_size;

    int array_which_child[allocator->levels];

    unsigned count;
    for(count = 0; index != 0; count++, index = parent_index(index))
    {
        array_which_child[count] = (right_child_index(parent_index(index)) == index);
    }

    for(; count >= 0; count--)
    {
        chunk_size >>= 1;
        if(array_which_child[count]) chunk += chunk_size;
    }

#if DEBUG_MODE
printf("----------------------------------------------------------------------------------\n\
Function:  index2address:\n\tallocator->buffer: %p\n\tindex: %u\n\taddress: %p\n\t\
address - buffer: %d\n----------------------------------------------------------------------------------\n"
, allocator->buffer, index, chunk, (int)(chunk - ((void*)allocator->buffer)) );
#endif

    return chunk;
}

/*
    Given a valid <address> in <allocator->buffer>, returns its corresponding
    index in <b_uddy_allocator_b_tree_address(allocator)>; if <address> is not 
    valid, returns <allocator->b_tree_size> 
*/

unsigned address2index(buddy_allocator* allocator, void* address)
{
    b_tree* tree = buddy_allocator_b_tree_address(allocator);
    unsigned offset = address - ((void*) buddy_allocator_b_tree_address(allocator));

#if DEBUG_MODE
printf("----------------------------------------------------------------------------------\n\
Function:  address2index:\n\tallocator->buffer: %p\n\taddress: %p\n\t\
offset: %d\n", allocator->buffer, address, (int)(address - ((void*) allocator->buffer)) );
#endif

    unsigned index = 0;
    unsigned current_offset = 0;
    unsigned current_size = allocator->buffer_size;

#if DEBUG_MODE
unsigned u = 0;
#endif

    for(; index < allocator->b_tree_size ;)
    {
        current_size >>= 1;
        
        if((offset == current_offset) && 
                        (b_tree_get(tree, left_child_index(index)) == FREE_FLAG))
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

#if DEBUG_MODE
printf("\tIteration n%u:\n\t\tindex: %u\n\t\tcurrent_offset: %u\n\t\t\
current_size: %u\n----------------------------------------------------------------------------------\n"
, u++, index, current_offset, current_size);
#endif

    }
    return allocator->b_tree_size;
}

unsigned buddy_allocator_memrequired(unsigned buffer_size, unsigned min_bucket_size)
{

#if DEBUG_MODE
printf("----------------------------------------------------------------------------------\n\
Function:  buddy_allocator_memrequired:\n\tbuffer_size: %u\n\tmin_bucket_size: %u\n\t\
mem_req: %u\n----------------------------------------------------------------------------------\n"
, buffer_size, min_bucket_size,(unsigned) (sizeof(buddy_allocator) + 
        b_tree_complete_memrequired(levels_needed(buffer_size, min_bucket_size))));
#endif

    return sizeof(buddy_allocator) + 
            b_tree_complete_memrequired(levels_needed(buffer_size, min_bucket_size));
}

int buddy_allocator_init(void* working_memory, unsigned working_memory_size,
                         char* buffer, unsigned buffer_size,
                         unsigned min_bucket_size)
{
    int error_code = 0;

#if DEBUG_MODE
printf("----------------------------------------------------------------------------------\n\
Function:  buddy_allocator_init:\n\tworking_memory: %p\n\tworking_memory_size: %u\n\t\
buffer: %p\n\tbuffer_size: %u\n\tmin_bucket_size: %u\n", working_memory, 
working_memory_size, buffer, buffer_size, min_bucket_size);
#endif

    /*
        If there's no room for a buddy_allocator struct in <working_memory>,
        we return an error code and abort.
    */

    if(buffer_size < (sizeof(buddy_allocator) + 1))
        return -1;

    /*
        We check if we can fit in <working_memory> a tree big enough to manage
        efficiently the <buffer>, given the minimum workload <min_bucket_size>
    */

    if(buffer_size < sizeof(buddy_allocator) + 
                                    b_tree_complete_memrequired(levels_needed(buffer_size, min_bucket_size)))
        error_code += 1;

    /*
        Initializing a buddy_allocator in <working_memory>
    */

    buddy_allocator* allocator = (buddy_allocator*) working_memory;
    
        allocator->buffer = buffer;
        allocator->buffer_size = buffer_size;
        allocator->b_tree_size = buddy_allocator_b_tree_nodes(working_memory_size);
        allocator->levels = fitting_levels(working_memory_size);

    /*
        Initializing the binary tree
    */

#if DEBUG_MODE
printf("\tb_tree_address %p\n\tb_tree_size: %u\n\
----------------------------------------------------------------------------------\n", 
buddy_allocator_b_tree_address(allocator), allocator->b_tree_size );
#endif

    b_tree_init(buddy_allocator_b_tree_address(allocator), allocator->b_tree_size);

    /*
        The whole buffer is free at the beginning
    */

    b_tree_put(buddy_allocator_b_tree_address(allocator), 0, FREE_FLAG);

    return error_code;
}

/*
    It works, but needs to be rewritten. It should be O(levels) but is actually O(2^levels)
*/
void* buddy_allocator_malloc(buddy_allocator* allocator, unsigned size)
{
    unsigned level = appropriate_level(allocator, size);
    b_tree* tree = buddy_allocator_b_tree_address(allocator);

    unsigned lim = min( allocator->b_tree_size, max_index_on_level(level) );
    
#if DEBUG_MODE
printf("----------------------------------------------------------------------------------\n\
Function:  buddy_allocator_malloc:\n\tsize: %u\n\tlimit_index: %u\n", size, lim );
#endif

    unsigned u;
    for(u = 0; u < lim; u++)
        if(b_tree_get(tree, u) == FREE_FLAG)
        {
            if(u == 0) break;
            unsigned index = u;
            /*
                Marking all the above levels as occupied
            */
            for(; (parent_index(u) != 0) && 
                            ( b_tree_get( tree, parent_index(u) ) != OCCUPIED_FLAG); u = parent_index(u))
                b_tree_put(tree, u, OCCUPIED_FLAG);
            
#if DEBUG_MODE
printf("Found index: %d\n\tcorresponding address: %p\n\
----------------------------------------------------------------------------------\n"
, u, index2address(allocator, index));
#endif
            
            return index2address(allocator, index);
        }

#if DEBUG_MODE
printf("\tNot enough memory\n\
----------------------------------------------------------------------------------\n");
#endif

    return NOT_ENOUGH_MEMORY(allocator);
}

//Will probably need to be rewritten as well
void buddy_allocator_free(buddy_allocator* allocator, void* address)
{
#if DEBUG_MODE
printf("----------------------------------------------------------------------------------\n\
Function:  buddy_allocator_free\t\naddress: %p\n", address);
#endif
    b_tree* tree = buddy_allocator_b_tree_address(allocator);
    unsigned index = address2index(allocator, address);

#if DEBUG_MODE
printf("\tcorresponding index: %u\n\
----------------------------------------------------------------------------------\n", index);
#endif

    /*
        If address2index returns an index equal to the length of the tree,
        then the address was not valid and we do nothing
    */
    
    if(index == allocator->b_tree_size)
        return;

    b_tree_put(tree, index, FREE_FLAG);

    /*
        We could do some clean-up for security reasons here, if needed.
        to be included as an option in the next version
    */

        //TODO

    /*
        If both an address and its buddy are now free, then we bring them back together
        and do the same with their parents
    */
    while((index != 0) && (b_tree_get(tree, sibling_index(index)) == FREE_FLAG))
    {
        index = parent_index(index);
        b_tree_put(tree, index, FREE_FLAG);
    }
}