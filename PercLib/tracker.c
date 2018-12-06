#include "perclib.h"

// Code for creating and managing Tracker structures.

struct Tracker newTracker(int s) {
	// To prevent false sharing, columns in different segments shouldn't appear on the same cache line.
	// So the allocation process starts by finding how cache lines are aligned.
	size_t cache = getCacheLine();

	// This method of allocating a 2D array was the main thing I learned from this project.
	// The whole thing occupies a single contiguous chunk of memory, requiring only one malloc() and one free().
	// The block starts with a header of pointers, each pointing to the beginning of a column containing actual data.
	size_t header = s * sizeof(bool*);
	size_t data = s * s * sizeof(bool);

	// The data section needs to be cache aligned as that is the part actually being modified.
	// So there may need to be an offset of empty memory between header and data.
	size_t offset = header % cache;

	bool** matrix = (bool**)_aligned_malloc(header + offset + data, cache);

	if (matrix == NULL) {
		printf("Unable to allocate Tracker matrix. Abort.\n");
		exit(EXIT_FAILURE);
	}

	// Now for the magic part. We take advantage of the fact that for double pointer p: p[x][y] is equivalent to *(*p + x) + y.
	// We just make sure *p + x points to the appropriate place in the data area, and the double pointer can be used as a 2D array.

	// First, get the adress of the beginning of the data area.
	bool* first = (bool*)((char*)matrix + header + offset);

	for (int x = 0; x < s; x++) {
		matrix[x] = first + x * s;
		
		for (int y = 0; y < s; y++) {
			matrix[x][y] = false;
		}
	}
	

	// Finally, find how many columns will form a cache aligned segment.
	// Start with 1 segment per available processor and decrement until aligned.
	int segSize = 1;
	for (int i = s / omp_get_max_threads(); i > 0; i--) {
		if (i * s * sizeof(bool) % cache == 0) {
			segSize = i;
			break;
		}
	}

	return (struct Tracker) { s, segSize, matrix };
}

// Print Tracker data to file.
void printTracker(struct Tracker tracker) {
	FILE* out;

	if (fopen_s(&out, "tracker.txt", "w") != 0) {
		printf("Unable to open file for printing.\n");
		return;
	}

	for (int y = 0; y < tracker.size; y++) {
		for (int x = 0; x < tracker.size; x++) {
			if (tracker.data[x][y]) {
				fprintf(out, "T ");
			}
			else {
				fprintf(out, "F ");
			}
		}

		fprintf(out, "\n");
	}

	fclose(out);
}