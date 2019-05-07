DEPS = src/binary_tree.h src/bitmap.c src/stack.h src/stack.c src/buddy_allocator.h src/buddy_allocator.c
SRC = src/bitmap.c src/stack.c src/buddy_allocator.c
CFLAGS = -O3

buddy_allocator: $(DEPS)
	gcc $(SRC) -c $(CFLAGS)

.PHONY: clean

clean:
	rm buddy_allocator.o bitmap.o stack.o
