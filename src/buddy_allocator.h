#define buddy_allocator_b_tree_address(allocator) (allocator + sizeof(buddy_allocator))

/*
    Given an amount (<buffer_size>) of memory (<buffer>) to manage, we do it as follows:
        when required to allocate a certain <amount> of memory, we divide <buffer> in 2^level
        parts, where level is such that buffer / 2^(level + 1) < <amount> and level <= <levels>.
*/

typedef struct buddy_allocator_s {
    char* buffer;
    unsigned buffer_size;
    unsigned levels;
} buddy_allocator;

/*
    The amount of memory (in bytes) needed for the optimal functioning of the buddy allocator.
    You should use this function to determine how much memory you should give to buddy_allocator_init
    as <working_memory>.

    The typical initialization of this allocator should be:

        char buffer[buffer_size];

        unsigned mem_required = buddy_allocator_memrequired(buffer_size, min_bucket_size);

        char working_memory[mem_required];

        buddy_allocator allocator = buddy_allocator_init(working_memory, mem_required
                                                         buffer, buffer_size, min_bucket_size);
*/

unsigned buddy_allocator_memrequired(unsigned buffer_size, unsigned min_bucket_size);

/* Initializes the buddy allocator with the following parameters:
     
    -working_memory : needs an empty buffer (the size of which can be computed
                       via the macro above) to fill with management structures

    -buffer : the actual memory to be managed

    -min_bucket_size: the minimum chunk that will be managed

    (the other parameters are self-evident)

   The returned integer stands for:

   -2147483647 : the working_memory_length was not enough to contain fundamental structs:
        the allocator will not work (segmentation fault, or worse, will occur if you try to use it) 

    0 : all good

        //NEED FURTHER EXPLANATIONS
    1 : the working_memory_length was not enough to contain the tree:
        the allocator will work, but there won't be as many "little chunks" as required
    
    //TODO

    (the error code is actually a mask of flags such that: if the most significant bit is 1, the error is
     beyond repair; if it's 0, it works but with some hindrances )
*/
int buddy_allocator_init(void* working_memory, unsigned working_memory_length,
                         char* buffer, unsigned buffer_size,
                         unsigned min_bucket_size);

//Nothing to say here
void* buddy_allocator_malloc(buddy_allocator* allocator, unsigned size);

//Nothing to say here
void buddy_allocator_free(buddy_allocator* allocator, void* address);