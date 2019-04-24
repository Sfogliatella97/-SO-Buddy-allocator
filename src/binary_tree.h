typedef void b_tree;

//Initialized a binary tree with as many levels as possible
b_tree btree_init(void* mem, unsigned length);

//Marks the chunk of memory represented by <index> as occupied
btree_lock(unsigned index);

//Finds the corresponding index in the tree and marks it as free
btree_release(void* mem);