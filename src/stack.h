/*
    A stack of unsigned int values
*/

/*
    We need define the functions in the header in order for the compiler to inline them properly
*/

#define stack_array(stack, index) ( ((unsigned*)( (char*)(stack) + sizeof(stack) ) + index) )

typedef struct stack_s {
    unsigned head_index;
    unsigned size;
} stack;

static inline unsigned stack_mem_required(unsigned max_n_elements)
{
        return ( sizeof(unsigned) * max_n_elements ) + sizeof(stack);
}

static inline stack* stack_init(char* mem)
{
        stack* s = (stack*) mem;
        s->head_index = 0;
        s->size = 0;
        return s;
}

static inline void stack_push(stack* s, unsigned val)
{
        *stack_array(s, s->head_index) = val;
        s->size++;
        s->head_index++;
}

static inline unsigned stack_pop(stack* s)
{
        s->size--;

        unsigned val = *stack_array(s, s->head_index - 1);
        s->head_index--;

        return val;
}

static inline int stack_is_empty(stack* s)
{
        return (s->size == 0);
}