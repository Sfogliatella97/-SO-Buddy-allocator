#include "binary_tree.h"

typedef char bitmap;

bitmap* bitmap_init(char* mem, unsigned length)
{
    unsigned u;
    for(u=0; u < length; u++)
        mem[u] = 0;
    return (bitmap*) mem;
}

void bitmap_put(bitmap* map, char index, int val)
{
    unsigned base = (index >> 3);
    unsigned offset = (index - (base << 3));
    char MASK = 1 << (7 - offset);
    if(val)
        map[base] |= MASK;
    else
    {
        MASK = ~MASK;
        map[base] &= MASK;
    }
}

int bitmap_get(bitmap* map, unsigned index)
{
    unsigned base = index >> 3;
    unsigned offset = index - (base << 3);
    char MASK = 1 << (7 - offset);
    char res = map[base] & MASK;
    return (res == MASK);
}

b_tree* b_tree_init(void* mem, unsigned length) 
{ (b_tree*) bitmap_init(mem, length); }

void b_tree_put(b_tree* tree, unsigned index, int val)
{ bitmap_put((bitmap*) tree, index, val); }

int b_tree_get(b_tree* tree, unsigned index)
{ bitmap_get((bitmap*) tree, index); }