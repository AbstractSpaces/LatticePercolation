#include "perclib.h"

// Code for creating and managing Tracker structures.

struct Tracker newTracker(int w, int s) {
	size_t cache = getCacheLine();

	// Using the same 2D allocation method as the lattice.
	size_t header = w * sizeof(bool*);
	size_t offset = header % cache;
	size_t data = w * s * sizeof(bool);

	bool** matrix = (bool**)_aligned_malloc(header + offset + data, cache);

	if (matrix == NULL) {
		printf("Unable to allocate Tracker matrix. Abort.\n");
		exit(EXIT_FAILURE);
	}

	bool* first = (bool*)((char)matrix + header + offset);

	for (int x = 0; x < w; x++) {
		matrix[x] = first + x * s;

		for (int y = 0; y < s; y++) {
			matrix[x][y] = false;
		}
	}

	return (struct Tracker) { w, s, matrix };
}