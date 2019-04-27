#include "buddy_allocator.h"
#include "binary_tree.h"

#define FREE_FLAG (1)
#define OCCUPIED_FLAG (!(FREE_FLAG))

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
    return levels;
}

/*
    Given the size of a buffer, computes the amount of levels we can fit in there
*/

static unsigned fitting_levels(unsigned working_memory_length)
{
    unsigned count= 1;
    while(((1 << count)-1) < working_memory_length)
        count++;
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
    return level;
}

/*
    Given an index in the binary tree, returns the corresponding address in the buffer.
    This could be a lot easier with the assumption that buffer_size = 2^levels.
    This needs to be addressed in the next version.
*/

void* index_to_address(buddy_allocator* allocator, unsigned index)
{
    void* chunk = allocator->buffer;
    unsigned chunk_size = allocator->buffer_size;

    unsigned count;
    char array_which_child[allocator->levels];

    for(count = 0; index != 0; count++, index = parent_index(index))
    {
        array_which_child[count] = (right_child_index(parent_index(index)) == index);
    }

    for(; count >= 0; count--)
    {
        chunk_size >>= 1;
        if(array_which_child[count]) chunk += chunk_size;
    }
    return chunk;
}

/*

*/

unsigned address_to_index(buddy_allocator* allocator, void* address)
{
    return 0;
}

int buddy_allocator_init(void* working_memory, unsigned working_memory_length,
                         char* buffer, unsigned buffer_size,
                         unsigned min_bucket_size)
{
    int error_code = 0;
    
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
        allocator->levels = fitting_levels(working_memory_length);

    /*
        Initializing the binary tree
    */

    b_tree_init(buddy_allocator_b_tree_address(allocator), working_memory_length);

    /*
        The whole buffer is free at the beginning
    */

    b_tree_put(buddy_allocator_b_tree_address(allocator), 0, FREE_FLAG);

    return error_code;
}

//It works, but needs to be rewritten. It should be O(levels) but is actually O(2^levels)
void* buddy_allocator_malloc(buddy_allocator* allocator, unsigned size)
{
    unsigned level = levels_needed(allocator->buffer_size, size);
    return 0;
}

//Will probably need to be rewritten as well
void buddy_allocator_free(buddy_allocator* allocator, void* address)
{
    b_tree* tree = buddy_allocator_b_tree_address(allocator);
    unsigned index = address_to_index(allocator, address);
    b_tree_put(tree, FREE_FLAG);

    //We could do some clean-up for security reasons here, if needed.
    //to be included as an option in the next version

    while((index) && (tree, sibling_index(index)) == FREE_FLAG))
    {
        index = parent_index(index);
        b_tree_put(tree, index, FREE_FLAG);
    }
}