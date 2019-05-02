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

unsigned bitmap_length(unsigned size)
{
    return size << 3;
}

unsigned bitmap_memrequired(unsigned length)
{

dbug_n("bitmap_memrequired");

    unsigned base = length >> 3;

dbug_formatted_print("\tlength: %u\n", length);
dbug_formatted_print("\tmemrequired: %u\n", ((length == (base << 3))? base : base + 1));

dbug_e("bitmap_memrequired");
    
    return ((length == (base << 3))? base : base + 1);
}

bitmap* bitmap_init(char* mem, unsigned length)
{
    unsigned u;
    unsigned size = length >> 3;
    for(u=0; u < size; u++)
    {
        mem[u] = 0;
        //printf("%u\n", u);
    }
    return (bitmap*) mem;
}

void bitmap_put(bitmap* map, unsigned index, int val)
{
    unsigned base = (index >> 3);
    unsigned offset = (index - (base << 3));
    char LOCAL_MASK = MASK[offset];
    if(val)
        map[base] |= LOCAL_MASK;
    else
    {
        LOCAL_MASK = ~LOCAL_MASK;
        map[base] &= LOCAL_MASK;
    }
}

int bitmap_get(bitmap* map, unsigned index)
{
    unsigned base = index >> 3;
    unsigned offset = index - (base << 3);
    char LOCAL_MASK = MASK[offset];
    char res = map[base] & LOCAL_MASK;
    return (res == LOCAL_MASK);
}

/*
    We use the b_tree as a layer of abstraction and call functions in binary_tree.h.
    If we compile buddy_allocator.c with bitmap.c (this file), we are actually using a bitmap,
    but buddy_allocator doesn't need to know that. So we wrap functions, hoping that the compiler
    will optimize it away.
*/

unsigned b_tree_length(unsigned size)
{ return bitmap_length(size); }

unsigned b_tree_memrequired(unsigned length)
{ return bitmap_memrequired(length); }

b_tree* b_tree_init(void* mem, unsigned length) 
{ return (b_tree*) bitmap_init(mem, length); }

void b_tree_put(b_tree* tree, unsigned index, int val)
{ bitmap_put((bitmap*) tree, index, val); }

int b_tree_get(b_tree* tree, unsigned index)
{ return bitmap_get((bitmap*) tree, index); }