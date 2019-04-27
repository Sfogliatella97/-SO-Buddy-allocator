#include "binary_tree.h"

typedef char bitmap;

unsigned bitmap_memrequired(unsigned size)
{
    unsigned base = size >> 3;
    if(size == (base << 3))
        return base;
    else 
        return (base + 1);
}

bitmap* bitmap_init(char* mem, unsigned length)
{
    unsigned u;
    for(u=0; u < length; u++)
        mem[u] = 0;
    return (bitmap*) mem;
}

void bitmap_put(bitmap* map, unsigned index, int val)
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

/*
    We use the b_tree as a layer of abstraction and call functions in binary_tree.h.
    If we compile buddy_allocator.c with bitmap.c (this file), we are actually using a bitmap,
    but buddy_allocator doesn't need to know that. So we wrap functions, hoping that the compiler
    will optimize it away.
*/

unsigned b_tree_memrequired(unsigned size)
{ return bitmap_memrequired(size);}

b_tree* b_tree_init(void* mem, unsigned length) 
{ return (b_tree*) bitmap_init(mem, length); }

void b_tree_put(b_tree* tree, unsigned index, int val)
{ bitmap_put((bitmap*) tree, index, val); }

int b_tree_get(b_tree* tree, unsigned index)
{ return bitmap_get((bitmap*) tree, index); }