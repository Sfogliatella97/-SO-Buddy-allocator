#include <stdio.h>
#include <stdlib.h>
#include "binary_tree.h"
#include "buddy_allocator.h"
#include "stack.h"
#include "debug_utilities.h"

#define TEST_1_BUFFER_SIZE 2048
#define TEST_1_MINBUCKET_SIZE 2

#define TEST_2_BUFFER_SIZE 9400
#define TEST_2_MINBUCKET_SIZE 2

#define buddy_allocator_b_tree_address(allocator) ( ((char*)(allocator)) + sizeof(buddy_allocator) )

/*
#define BUFFER_SIZE 30000
#define MIN_BUCKET_SIZE 20
*/


#define failure(msg)  do {\
                            fprintf(stderr, "%s\n", msg);\
                            exit(EXIT_FAILURE);\
                            }while(0);

void insertion_sort(int** array, unsigned length)
{
    unsigned max;
    unsigned u, j;
    for(u = length - 1; u != 0; u--)
    {
        max = u;
        for(j = 0; j < u; j++)
            if(array[j] > array[max])
                max = j;
        int* tmp = array[max];
        array[max] = array[u];
        array[u] = tmp;
    }
}

void tree_print(b_tree* tree, unsigned length)
{
    unsigned u;
    unsigned lvl = 0;
    for(u = 0; u < length; u++)
    {
        printf("%d", b_tree_get(tree, u));
        if(u == max_index_on_level(lvl)) {printf("\n"); lvl++;}
    }
    printf("\n");
}

static void buddy_allocator_print(buddy_allocator* allocator)
{
    printf("Allocator:\n\tBuffer size: %u\n\tTree length: %u\n\tLevels: %u\n\tBiggest available chunk: %u\n\n"
    , allocator->buffer_size, allocator->b_tree_length, allocator->levels, buddy_allocator_available_mem(allocator));

}

int verify_condition(b_tree* tree, unsigned length)
{
    unsigned u;
    for(u = 0; u < length; u++)
        if( b_tree_get(tree, u) == 0)
            for(; u != 0; u = parent_index(u))
                if(b_tree_get(tree, u) != 0)
                    return 0;
    return 1;
}

int main()
{
        int                errnum;

        unsigned           u;
        unsigned           j;
        unsigned           working_mem_size_test_1;
        unsigned           working_mem_size_test_2;

        char               buffer_test_1[TEST_1_BUFFER_SIZE];
        buddy_allocator*   allocator_test_1;

        char               buffer_test_2[TEST_2_BUFFER_SIZE];
        buddy_allocator*   allocator_test_2;

        printf("Beginning tests:\n");


        printf("> Creating allocator with buffer size %u and min bucket size %u:\n", TEST_1_BUFFER_SIZE, TEST_1_MINBUCKET_SIZE);

        working_mem_size_test_1 = buddy_allocator_memrequired(TEST_1_BUFFER_SIZE, TEST_1_MINBUCKET_SIZE);

        char working_mem_test_1[working_mem_size_test_1];

        errnum           = buddy_allocator_init(&allocator_test_1,
                                                working_mem_test_1, working_mem_size_test_1, buffer_test_1
                                                , TEST_1_BUFFER_SIZE, TEST_1_MINBUCKET_SIZE);
        if(errnum != 0)    failure("buddy_allocator_memrequired is not working properly");

        buddy_allocator_print(allocator_test_1);

        printf("\nAllocator initialized correctly\n\n");

        short  tests[TEST_1_BUFFER_SIZE/TEST_1_MINBUCKET_SIZE];
        short* addresses[TEST_1_BUFFER_SIZE/TEST_1_MINBUCKET_SIZE];

        printf("\n\n>We check that addresses assigned don't overlap by allocating 1024 pointers to \n"
               " short integers and assigning them increasing values. When we are finished allocating,\n"
               " we compare their content with a test array\n\n");

        printf("Allocating %u integers:\n\n", TEST_1_BUFFER_SIZE / TEST_1_MINBUCKET_SIZE);

        for(u = 0; u < TEST_1_BUFFER_SIZE/TEST_1_MINBUCKET_SIZE; u++)
        {
                addresses[u] = buddy_allocator_malloc(allocator_test_1, 2);
                if(addresses[u] == NOT_ENOUGH_MEMORY(allocator_test_1)) failure("Error: failed to allocate memory, when there should be enough");
                *(addresses[u]) = (short)u;
                tests[u] = (short)u;
        }

        printf("Addresses allocated correctly\n\n");


        if( buddy_allocator_available_mem(allocator_test_1) != 0) failure("Error: the whole buffer should be allocated, but"
                                                                        " the allocator thinks there's space available\n\n");

        for(u = 0; u < TEST_1_BUFFER_SIZE/TEST_1_MINBUCKET_SIZE; u++)
        {
                if(*(addresses[u]) != tests[u])
                        failure("Error: Memory corruption detected");
        }
                                                                
        printf("No memory corruption detected\n\n");

        for( u = 0; u < TEST_1_BUFFER_SIZE/TEST_1_MINBUCKET_SIZE; u++)
        {
                buddy_allocator_free(allocator_test_1, addresses[u]);
        }

        if( buddy_allocator_available_mem(allocator_test_1) != TEST_1_BUFFER_SIZE) failure("Free is not working correctly");

        printf("Memory correctly released\n\n");

        printf("Test 1 passed\n\n");

        //------------------------------------------------------------------------------------------------------------

        printf("We check that there are no memory leaks: If a user allocates some memory and then release it\n"
               " correctly, no memory should \"disappear\"(This could happen if buddy_allocator_free() didn't "
               "restore correctly the bitmap)\n\n");

        printf("Creating allocator with buffer size %u and min bucket size %u:\n", TEST_2_BUFFER_SIZE, TEST_2_MINBUCKET_SIZE);

        working_mem_size_test_2 = buddy_allocator_memrequired(TEST_2_BUFFER_SIZE, TEST_2_MINBUCKET_SIZE);

        char working_mem_test_2[working_mem_size_test_2];

        errnum           = buddy_allocator_init(&allocator_test_2,
                                                working_mem_test_2, working_mem_size_test_2, buffer_test_2
                                                , TEST_2_BUFFER_SIZE, TEST_2_MINBUCKET_SIZE);
        if(errnum != 0)    failure("buddy_allocator_memrequired is not working properly");

        buddy_allocator_print(allocator_test_2);

        char* addresses_2[TEST_2_BUFFER_SIZE / TEST_2_MINBUCKET_SIZE];


        for( u = 0; u < 15; u++)
        {
                printf("Iteration n: %u\n", u);
                
                for(j = 0; j < TEST_2_BUFFER_SIZE / TEST_2_MINBUCKET_SIZE; j++)
                {
                        addresses_2[j] = buddy_allocator_malloc(allocator_test_2, TEST_2_MINBUCKET_SIZE);
                        if(addresses_2 == NOT_ENOUGH_MEMORY(allocator_test_2)) failure("Error: not enough memory, when there should be");
                }

                printf("allocating...\n");
                for(; j != 0; j--)
                        buddy_allocator_free(allocator_test_2, addresses_2[j-1]);

                printf("deallocating...\n\n");

                if(buddy_allocator_available_mem(allocator_test_2) != TEST_2_BUFFER_SIZE) failure("Error: memory leak");
            
        }

        printf("\nTest 2 passed\n");
}