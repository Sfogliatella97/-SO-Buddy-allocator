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

/*
    The number of nodes a tree of size <size> (in bytes) can store
*/

unsigned b_tree_length(unsigned size);

/*
    The memory (in bytes) required to store a tree with <size> nodes
*/

unsigned b_tree_memrequired(unsigned length);

/*
    Initializes a binary tree with as many levels as possible
*/

b_tree* b_tree_init(void* mem, unsigned length);

/*
    Puts <val> at index <index>
*/
void b_tree_put(b_tree* tree, unsigned index, int val);

/*
    Reads the value at index <index>
*/

int b_tree_get(b_tree* tree, unsigned index);