#include "lib.h"

// Functions for working with cluster data structures.

// Merge two clusters into one.
// The ordering of the arguments is important and needs to reflect their relative positions in the lattice.
struct Cluster stitch_clusters(struct Cluster left, struct Cluster right, int* num_clusters) {
	*num_clusters++;

	return (struct Cluster) {
		*num_clusters,
		left.size + right.size,
		left.x_min,
		right.x_max,
		(left.y_min < right.y_min) ? left.y_min : right.y_min,
		(left.y_max > right.y_max) ? left.y_max : right.y_max
	};
}

// My decision to use C and forgoe templates is biting pretty hard right now.
// Pretty much just copy pasting the stack code.
struct Cluster_List new_cluster_list(int s) {
	size_t cache = get_cache_line();
	size_t bytes = s * sizeof(struct Cluster);
	struct Cluster* data = (struct Cluster*)_aligned_malloc(bytes, cache);

	if (data == NULL) {
		printf("List allocation failure. Abort.\n");
		exit(EXIT_FAILURE);
	}

	return (struct Cluster_List) { 0, s, data };
}

void push_cluster(struct Cluster_List* list, struct Cluster new) {
	if (list->size + 1 > list->max) {
		list->max *= 2;
		size_t cache = get_cache_line();
		size_t bytes = list->max * sizeof(struct Cluster);
		list->data = (struct Cluster*)_aligned_realloc(list->data, bytes, cache);

		if (list->data == NULL) {
			printf("Unable to resize list. Abort.\n");
			exit(EXIT_FAILURE);
		}
	}

	list->data[list->size] = new;
	list->size++;
}

void print_cluster_list(struct Cluster_List list) {
	printf("\nClusters:\n\n");
	for (int i = 0; i < list.size; i++) {
		struct Cluster c = list.data[i];
		printf("ID: %d\n", c.id);
		printf("Size: %d\n", c.size);
		printf("X Range: %d - %d\n", c.x_min, c.x_max);
		printf("Y Range: %d - %d\n", c.y_min, c.y_max);
		printf("\n");
	}
}