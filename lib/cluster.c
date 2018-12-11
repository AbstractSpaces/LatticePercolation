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