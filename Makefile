DEPS = src/binary_tree.h src/bitmap.c src/buddy_allocator.h src/buddy_allocator.c
SRC = src/bitmap.c src/buddy_allocator.c
CFLAGS = -Wall 

test: $(DEPS) src/test.c src/stack.h src/stack.c
	gcc src/test.c src/stack.c $(SRC) -o test -Wall -g -std=c11

buddy_allocator: $(DEPS)
	gcc $(SRC) -c $(CFLAGS)
