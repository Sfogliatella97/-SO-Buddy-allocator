# -SO-Buddy-allocator
Buddy allocator for the exam "Sistemi operativi" in Sapienza 

To use this allocator you should:
1. clone it
2. make it
3. move buddy_allocator.h to your headers folder and include it wherever you need
4. move buddy_allocator.o to your binaries folder
5. Compile your programs with buddy_allocator.o

It is advised that you initialize your buddy_allocator like this:


        buddy_allocator* allocator;

        char buffer[buffer_size];

        unsigned mem_required = buddy_allocator_memrequired(buffer_size, min_bucket_size);

        char working_memory[mem_required];

        int err = buddy_allocator_init(allocator, working_memory, mem_required, buffer, buffer_size, min_bucket_size);
        
        if(err == ... //checks on err

        else 
            ....
