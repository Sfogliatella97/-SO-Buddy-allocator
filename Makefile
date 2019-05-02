DEPS = src/binary_tree.h src/bitmap.c src/buddy_allocator.h src/buddy_allocator.c
SRC = src/bitmap.c src/buddy_allocator.c
CFLAGS = -Wall 

test: src/test.c src/debug_utilities.h $(DEPS) 
	gcc -o test src/test.c $(SRC) -g -std=c11

buddy_allocator: $(DEPS)
	gcc $(SRC) -c $(CFLAGS)