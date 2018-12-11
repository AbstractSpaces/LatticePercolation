#include "lib.h"

// Functions for creating and managing stack structures.

struct Stack new_stack(int s) {
	// Parallel threads will each have their own stack, so stacks should be cache aligned to avoid false sharing.
	size_t cache = get_cache_line();
	size_t bytes = s * sizeof(struct Vertex);
	struct Vertex* data = (struct Vertex*)_aligned_malloc(bytes, cache);

	if (data == NULL) {
		printf("Stack allocation failure. Abort.\n");
		exit(EXIT_FAILURE);
	}

	return (struct Stack) { 0, s, data };
}

void push_stack(struct Stack* stack, struct Vertex new) {
	// Allocate extra memory if necessary.
	if (stack->size + 1 > stack->max) {
		stack->max *= 2;
		size_t cache = get_cache_line();
		size_t bytes = stack->max * sizeof(struct Vertex);
		stack->data = (struct Vertex*)_aligned_realloc(stack->data, bytes, cache);

		if (stack->data == NULL) {
			printf("Unable to resize stack. Abort.\n");
			exit(EXIT_FAILURE);
		}
	}

	stack->data[stack->size] = new;
	stack->size++;
}

struct Vertex pop_stack(struct Stack* stack) {
	if (stack->size < 1) {
		printf("Trying to pop empty stack. Abort. \n");
		// Yes this is a new level of lazy error handling. But it makes debugging a whole lot easier.
		exit(EXIT_FAILURE);
	}

	stack->size--;
	return stack->data[stack->size];
}