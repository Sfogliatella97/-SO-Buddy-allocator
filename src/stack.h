/*
    A stack of unsigned int values
*/

typedef void stack;

unsigned stack_mem_required(unsigned max_n_elements);

stack* stack_init(char* mem);

void stack_push(stack* stack, unsigned val);

unsigned stack_pop(stack* stack);

int stack_is_empty(stack* stack);