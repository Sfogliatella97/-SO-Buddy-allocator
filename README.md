# -SO-Buddy-allocator
Buddy allocator for the exam "Sistemi operativi" in Sapienza 

It should be functioning.

To use this allocator you should:
1. clone it
2. move to \\...\\-SO-Buddy-allocator
3. call 'make buddy_allocator'
4. move bitmap.o, buddy_allocator.o to where your object files are
5. move buddy_allocator.h to where your headers are
6. include buddy_allocator.h
7. compile with bitmap.o and buddy_allocator.o

It is advised that you initialize your buddy_allocator like this:


        buddy_allocator* allocator;

        char buffer[buffer_size];

        unsigned mem_required = buddy_allocator_memrequired(buffer_size, min_bucket_size);

        char working_memory[mem_required];

        int err = buddy_allocator_init(working_memory, mem_required
                                                         buffer, buffer_size, min_bucket_size);
        
        if(err == ... //checks on err

        else 
            allocator = (buddy_allocator*) working_memory;
