#include "binary_tree.h"

#include "debug_utilities.h"

/*
    Only works if a char is 8 bit
*/

typedef char bitmap;

char MASK[8] = {
    1 << 7,
    1 << 6,
    1 << 5,
    1 << 4,
    1 << 3,
    1 << 2,
    1 << 1,
    1 
};

char NOT_MASK[8] = {
    ~ (1 << 7),
    ~ (1 << 6),
    ~ (1 << 5),
    ~ (1 << 4),
    ~ (1 << 3),
    ~ (1 << 2),
    ~ (1 << 1),
    ~ 1
};

unsigned bitmap_fitting_length(unsigned size)
{
    return size << 3;
}

unsigned bitmap_memrequired(unsigned length)
{
    unsigned base = length >> 3;
    
    return ( (length == (base << 3))? (base) : (base + 1) );
}

bitmap* bitmap_init(char* mem, unsigned length)
{
    return (bitmap*) mem;
}

void bitmap_put(bitmap* map, unsigned index, int val)
{
    unsigned base = (index >> 3);
    unsigned offset = (index - (base << 3));

    if(val)
        map[base] |= MASK[offset];
    else
        map[base] &= NOT_MASK[offset];
}

int bitmap_get(bitmap* map, unsigned index)
{
    unsigned base = index >> 3;
    unsigned offset = index - (base << 3);

    char res = map[base] & MASK[offset];
    return (res == MASK[offset]);
}

/*
    We use the b_tree as a layer of abstraction and call functions in binary_tree.h.
    If we compile buddy_allocator.c with bitmap.c (this file), we are actually using a bitmap,
    but buddy_allocator doesn't need to know that. So we wrap functions, hoping that the compiler
    will optimize it away.
*/

unsigned b_tree_fitting_length(unsigned size)
{ return bitmap_fitting_length(size); }

unsigned b_tree_memrequired(unsigned length)
{ return bitmap_memrequired(length); }

b_tree* b_tree_init(void* mem, unsigned length) 
{ return (b_tree*) bitmap_init(mem, length); }

void b_tree_put(b_tree* tree, unsigned index, int val)
{ bitmap_put((bitmap*) tree, index, val); }

int b_tree_get(b_tree* tree, unsigned index)
{ return bitmap_get((bitmap*) tree, index); }