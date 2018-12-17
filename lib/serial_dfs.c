#include "lib.h"

struct Cluster_List serial_dfs(struct Lattice lat) {
	// Obtain the structures necessary for the search.
	struct Tracker track = new_tracker(lat.size);
	struct Stack stack = new_stack(lat.size);
	struct Cluster_List clusters = new_cluster_list(lat.size);

	// Start a depth first search at each unexplored site.
	for (int x = 0; x < lat.size; x++) {
		for (int y = 0; y < lat.size; y++) {
			// Trying not to nest the main algorithm too deep inside if blocks, hence the use of continue.
			// Skip if explored.
			if (track.data[x][y]) {
				continue;
			}

			// Skip if empty.
			if (check_empty(lat.data[x][y])) {
				track.data[x][y] = true;
				continue;
			}

			push_stack(&stack, (struct Vertex) { x, y });
			struct Cluster clust = { clusters.size, 0, x, x, y, y };

			while (stack.size > 0) {
				struct Vertex v = pop_stack(&stack);
				// Skip if already explored.
				if (track.data[v.x][v.y]) {
					continue;
				}

				// Update the cluster record.
				clust.size++;

				if (v.x > clust.x_max) {
					clust.x_max = v.x;
				}
				if (v.x < clust.x_min) {
					clust.x_min = v.x;
				}
				if (v.y > clust.y_max) {
					clust.y_max = v.y;
				}
				if (v.x < clust.x_min) {
					clust.y_min = v.y;
				}

				// Mark this Site as explored.
				track.data[v.x][v.y] = true;

				// Add bonded Sites to the stack.
				if (lat.data[v.x][v.y].up) {
					push_stack(&stack, (struct Vertex) { v.x, v.y - 1 });
				}
				if (lat.data[v.x][v.y].down) {
					push_stack(&stack, (struct Vertex) { v.x, v.y + 1 });
				}
				if (lat.data[v.x][v.y].left) {
					push_stack(&stack, (struct Vertex) { v.x - 1, v.y});
				}
				if (lat.data[v.x][v.y].right) {
					push_stack(&stack, (struct Vertex) { v.x + 1, v.y});
				}
			}

			push_cluster(&clusters, clust);
		}
	}

	_aligned_free(track.data);
	_aligned_free(stack.data);

	return clusters;
}