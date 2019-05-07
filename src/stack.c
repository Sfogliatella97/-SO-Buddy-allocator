#include "stack.h"

#include "debug_utilities.h"

#define stack_array(stack, index) ( ((unsigned*)( (char*)(stack) + sizeof(stack) ) + index) )

typedef struct stack_s {
    unsigned head_index;
    unsigned size;
} stack_u;

unsigned stack_u_mem_required(unsigned max_n_elements)
{
    return ( sizeof(unsigned) * max_n_elements ) + sizeof(stack_u);
}

stack_u* stack_u_init(char* mem)
{
    stack_u* stack = (stack_u*) mem;
    stack->head_index = 0;
    stack->size = 0;
    return stack;
}

void stack_u_push(stack_u* stack, unsigned val)
{
    *stack_array(stack, stack->head_index) = val;
    stack->size++;
    stack->head_index++;
}

unsigned stack_u_pop(stack_u* stack)
{
    stack->size--;

    unsigned val = *stack_array(stack, stack->head_index - 1);
    stack->head_index--;

    return val;
}

int stack_u_is_empty(stack_u* stack)
{
    return (stack->size == 0);
}

unsigned stack_mem_required(unsigned max_n_elements)
{ return stack_u_mem_required(max_n_elements); }

stack* stack_init(char* mem)
{ return stack_u_init(mem); }

void stack_push(stack* stack, unsigned val)
{ stack_u_push((stack_u*) stack, val); }

unsigned stack_pop(stack* stack)
{ return stack_u_pop((stack_u*) stack); }

int stack_is_empty(stack* stack)
{ return stack_u_is_empty((stack_u*) stack); }