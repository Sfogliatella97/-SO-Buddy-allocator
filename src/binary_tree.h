/*
    Macros to navigate the binary tree
*/

#define is_even(n) ((n) == (((n) >> 1) << 1))

#define parent_index(index) ((is_even( (index) +1 )? ( ( (index) +1) >> 1) : ((index) >> 1)) - 1)

#define right_child_index(index) ( ( (index) + 1) << 1)

#define left_child_index(index) (right_child_index(index) -1)

#define sibling_index(index) ( ( is_even((index) + 1) )? ((index) + 1) : ((index) - 1))

#define max_index_on_level(level) ((1 << ((level)+1)) -2)

/*
    The memory (in bytes) required to store a tree of <levels> number of levels,
    where 0 means only the root, 1 means the root and its 2 children,
    3 means the root, its 2 children and their 4 children, and so on
*/

#define b_tree_complete_memrequired(levels) (b_tree_memrequired(max_index_on_level(levels) + 1))

/*
    A binary tree with unsigned keys and int values.
    It should be managed with functions within this header and not be
    messed with otherwise
*/

typedef void b_tree;



typedef char bitmap;

/*
    Only works if a char is 8 bit
*/
#define MASK(offset) (1 << (7 - offset))

/*
    The number of nodes a tree of size <size> (in bytes) can store
*/

static inline unsigned b_tree_fitting_length(unsigned size)
{
    return ((size) << 3);
}

/*
    The memory (in bytes) required to store a tree with <size> nodes
*/

static inline unsigned b_tree_memrequired(unsigned length)
{
    unsigned base = length >> 3;
    
    return ( (length == (base << 3))? (base) : (base + 1) );
}

/*
    Initializes a binary tree with as many levels as possible
*/

static inline b_tree* b_tree_init(void* mem, unsigned length)
{
    return (b_tree*) ( (bitmap*)mem );
}

/*
    Puts <val> at index <index>
*/
static inline void b_tree_put(b_tree* tree, unsigned index, int val)
{
    bitmap* map = (bitmap*) tree;
    unsigned base = (index >> 3);
    unsigned offset = (index - (base << 3));

    char m = MASK(offset);

    if(val)
        map[base] |= m;
    else
        map[base] &= ~ m;
}

/*
    Reads the value at index <index>
*/

static inline int b_tree_get(b_tree* tree, unsigned index)
{
    bitmap* map = (bitmap*) tree;
    unsigned base = index >> 3;
    unsigned offset = index - (base << 3);

    char m = MASK(offset);

    char res = map[base] & m;
    return (res == m);
}