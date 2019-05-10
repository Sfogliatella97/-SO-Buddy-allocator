DEPS = src/binary_tree.h src/stack.h src/buddy_allocator.h src/buddy_allocator.c
SRC = src/buddy_allocator.c
CFLAGS = -O3

buddy_allocator: $(DEPS)
	gcc $(SRC) -c $(CFLAGS)

.PHONY: clean

clean:
	rm buddy_allocator.o
