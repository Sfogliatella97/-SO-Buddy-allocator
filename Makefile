DEPS = src/binary_tree.h src/bitmap.c src/buddy_allocator.h src/buddy_allocator.c
SRC = src/bitmap.c src/buddy_allocator.c
CFLAGS = -Wall 

buddy_allocator: $(DEPS)
	gcc $(SRC) -c $(CFLAGS)
