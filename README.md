# -SO-Buddy-allocator

Buddy allocator for the exam "Sistemi operativi" in Sapienza. It can manage buffers of any size (not necessarily
a power of 2) and with any working memory (it will always create as many levels as possible), as long as it is enough
to hold the fundamental data structures.
# Internals

The allocator keeps track of allocated blocks with a binary tree of boolean values: if the value is 1, then the
corresponding block is free; if it is 0, then the block is already allocated (There are exceptions to this,
see the source of unsigned greatest_available_index(buddy_allocator* allocator, unsigned lim) in src/buddy_allocator.c).
More details are in the comments.
# Use

To use this allocator you should:
1. clone it
2. make it
3. move buddy_allocator.h to your headers folder and include it wherever you need
4. move buddy_allocator.o to your binaries folder
5. Compile your programs with buddy_allocator.o

It is advised that you initialize your buddy_allocator more or less like this:


        buddy_allocator*   allocator;
        unsigned           mem_required;
        int                err;
        
        char buffer[buffer_size];
        char working_memory[mem_required];

        allocator = buddy_allocator_memrequired(buffer_size, min_bucket_size);
        err       = buddy_allocator_init(allocator, working_memory, mem_required, buffer, buffer_size, min_bucket_size);
        
        if(err == ... //checks on err

        else 
            ....
