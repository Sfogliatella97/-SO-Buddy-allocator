#define is_even(n) ((n == ((n >> 1) << 1))? 1 : 0)

#define parent_index(index) ((((is_even(index+1))? (index + 1 ) : (index)) << 1) -1)

#define right_child_index(index) ((index + 1) >> 1)

#define left_child_index(index) (right_child_index(index) -1)

typedef void b_tree;

//Initializes a binary tree with as many levels as possible
b_tree* b_tree_init(void* mem, unsigned length);

//Puts <val> in the tree at index <index>
void b_tree_put(b_tree* tree, unsigned index, int val);

int b_tree_get(b_tree* tree, unsigned index);