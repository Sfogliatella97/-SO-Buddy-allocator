#define NOT_ENOUGH_MEMORY(allocator) ((void*)  ((allocator)->buffer + (allocator)->buffer_size))

/*
    This struct will be placed inside <working_memory> by buddy_allocator_init. The tree
    takes the whole remaining part of <working_memory>.
*/

typedef struct buddy_allocator_s {
        char* buffer;             // Pointer to the managed memory 
        unsigned buffer_size;     // Size of the managed memory
        unsigned b_tree_length;   // Length of the binary tree which keeps track of allocated blocks
        unsigned levels;          // Number of levels of the tree
} buddy_allocator;

/*
    The amount of memory (in bytes) needed for the optimal functioning of the buddy allocator.
    You should use this function to determine how much memory you should give to buddy_allocator_init
    as <working_memory>.

    The typical initialization of this allocator should be:

            buddy_allocator*   allocator;
            unsigned           mem_required;
            int                err;
            char               buffer[buffer_size];
            char               working_memory[];

            memrequired = buddy_allocator_memrequired(buffer_size, min_bucket_size);
            
            working_memory[memrequired];
            
            err       = buddy_allocator_init(allocator, working_memory, mem_required, buffer, buffer_size, min_bucket_size);
    
            if(err == ... //checks on err

            else 
               ...
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

        0            : 
            all good

        1            : 
            the working_memory_size was not enough to contain a tree capable 
            of managing chunk of size <min_bucket_size>: the allocator will work, 
            but there will be internal fragmentation
    
        (1 << 32) + 1: 
            <working_mem_size> is too small to contain fundamental data structures

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
    On successfull call, makes <address> allocatable again;
    on unsuccessfull call, does nothing
*/
void buddy_allocator_free(buddy_allocator* allocator, void* address);

/*
    The biggest available contigous chunk of memory 
    (there could be more memory available, but it would not be not guaranteed to be contigous)
*/
unsigned buddy_allocator_available_mem(buddy_allocator* allocator);
