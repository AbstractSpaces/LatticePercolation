#include "percmain.h"

// Functions for creating and managing stack structures.

struct Stack newStack(int s) {
	// Parallel threads will each have their own stack, so stacks should be cache aligned to avoid false sharing.
	size_t cache = getCacheLine();

	struct Vertex* data = _aligned_malloc(s, cache);

	if (data == NULL) {
		printf("Stack allocation failure. Abort.\n");
		exit(EXIT_FAILURE);
	}

	return (struct Stack) { 0, s, data };
}

void pushStack(struct Stack stack, struct Vertex new) {
	// Allocate extra memory if necessary.
	if (stack.size + 1 > stack.max) {
		stack.max *= 2;
		stack.data = (struct Vertex*)_aligned_realloc(stack.data, stack.max, getCacheLine());

		if (stack.data == NULL) {
			printf("Unable to resize stack. Abort.\n");
			exit(EXIT_FAILURE);
		}
	}

	stack.data[stack.size] = new;
	stack.size += 1;
}

struct Vertex popStack(struct Stack stack) {
	if (stack.size < 1) {
		printf("Trying to pop empty stack. Abort. \n");
		// Yes this is a new level of lazy error handling. But it makes debugging a whole lot easier.
		exit(EXIT_FAILURE);
	}

	stack.size -= 1;
	return stack.data[stack.size];
}