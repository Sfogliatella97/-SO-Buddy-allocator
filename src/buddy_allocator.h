/*
#define buddy_allocator_size(buffer_size)  TODO
/*

/* Initializes the buddy allocator with the following parameters:
     
    -buddy_allocator : needs an empty buffer (the size of which can be computed
                       via the macro above) to fill with management structures

    -buffer : the actual memory to be managed

    -min_bucket_size: the minimum chunk that will be managed

    (the other parameters are self-evident)

   The returned integer stands for:

    0 : all good

    TODO
*/
int buddy_allocator_init(void* buddy_allocator,
                         char* buffer, unsigned buffer_size,
                         unsigned min_bucket_size);

//Nothing to say here
void* buddy_allocator_malloc(buddy_allocator* allocator, unsigned size);

//Nothing to say here
void* buddy_allocator_free(buddy_allocator* allocator, void* mem);