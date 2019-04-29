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

unsigned bitmap_memrequired(unsigned size)
{

dbug_n("bitmap_memrequired");

    unsigned base = size >> 3;

dbug_formatted_print("\tsize: %u\n", size);
dbug_formatted_print("\tmemrequired: %u\n", ((size == (base << 3))? base : base + 1));

dbug_e("bitmap_memrequired");
    
    return ((size == (base << 3))? base : base + 1);
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

unsigned b_tree_memrequired(unsigned size)
{ return bitmap_memrequired(size);}

b_tree* b_tree_init(void* mem, unsigned length) 
{ return (b_tree*) bitmap_init(mem, length); }

void b_tree_put(b_tree* tree, unsigned index, int val)
{ bitmap_put((bitmap*) tree, index, val); }

int b_tree_get(b_tree* tree, unsigned index)
{ return bitmap_get((bitmap*) tree, index); }