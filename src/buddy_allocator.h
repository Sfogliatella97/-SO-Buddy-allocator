#define NOT_ENOUGH_MEMORY(allocator) ((void*)  ((allocator)->buffer + (allocator)->buffer_size))

/*
    Given an amount (<buffer_size>) of memory (<buffer>) to manage, we do it as follows:
        when required to allocate a certain <amount> of memory, we divide <buffer> in 2^level
        parts, where level is such that buffer / 2^(level + 1) < <amount> and level <= <levels>.
*/

typedef struct buddy_allocator_s {
    char* buffer;
    unsigned buffer_size;
    unsigned b_tree_length;
    unsigned levels;
} buddy_allocator;

/*
    The amount of memory (in bytes) needed for the optimal functioning of the buddy allocator.
    You should use this function to determine how much memory you should give to buddy_allocator_init
    as <working_memory>.

    The typical initialization of this allocator should be:

        char buffer[buffer_size]; ( Maybe you should add __attribute__((aligned(64)))), but I'm not sure about that)

        unsigned mem_required = buddy_allocator_memrequired(buffer_size, min_bucket_size);

        char working_memory[mem_required];

        buddy_allocator* allocator;

        int err = buddy_allocator_init(allocator, working_memory, mem_required, buffer, buffer_size, min_bucket_size);
        
        if(err == ... //checks on err

        else 
            allocator = (buddy_allocator*) working_memory;
*/

unsigned buddy_allocator_memrequired(unsigned buffer_size, unsigned min_bucket_size);

/* Initializes the buddy allocator with the following parameters:
     
    -working_memory : needs an empty buffer (the size of which can be computed
                       via the macro above) to fill with management structures

    -buffer : the actual memory to be managed

    -min_bucket_size: the minimum chunk that will be managed

    (the other parameters are self-evident)

   The returned integer stands for:

    First and last bytes are 1 : the working_memory_size was not enough to contain fundamental structs:
        the allocator will not work (segmentation fault, or worse, will occur if you try to use it anyway) 

    0 : all good

    1 : the working_memory_size was not enough to contain a tree capable of managing chunk of size <min_bucket_size>:
        the allocator will work, but there will be internal fragmentation
    
    (1 << 32) + 1: <working_mem_size> is too small to contain fundamental data structures

    (the error code is such that: if the most significant bit is 1, the error is
     beyond repair; if it's 0, it works but with some hindrances )
*/
int buddy_allocator_init(buddy_allocator** allocator, void* working_memory, 
                         unsigned working_memory_size, char* buffer, unsigned buffer_size,
                         unsigned min_bucket_size);

/*
    On successfull call, returns an appropriate address in <allocator->buffer>; 
    on unsuccessfull call, returns <NOT_ENOUGH_MEMORY(allocator)> (which is always
    out of <allocator->buffer>). Whenever you call this function, you should check its value:

    void* address = buddy_allocator_malloc(allocator, size);

    if(address == NOT_ENOUGH_MEMORY(allocator))
        //ERROR HANDLING
*/
void* buddy_allocator_malloc(buddy_allocator* allocator, unsigned size);

/*
    On successfull call, makes <address> allocable again:
    on unsuccessfull call, does nothing
*/
void buddy_allocator_free(buddy_allocator* allocator, void* address);