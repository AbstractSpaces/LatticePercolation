#include "perclib.h"

// Code for creating and managing Tracker structures.

struct Tracker newTracker(int s) {
	size_t cache = getCacheLine();

	// Using the same 2D allocation method as the lattice.
	// Only not utterly unnecessarily this time.
	// Idiot.
	size_t header = s * sizeof(bool*);
	size_t offset = header % cache;
	size_t data = s * s * sizeof(bool);

	bool** matrix = (bool**)_aligned_malloc(header + offset + data, cache);

	if (matrix == NULL) {
		printf("Unable to allocate Tracker matrix. Abort.\n");
		exit(EXIT_FAILURE);
	}

	bool* first = (bool*)((char)matrix + header + offset);

	for (int x = 0; x < s; x++) {
		matrix[x] = first + x * s;
		
		for (int y = 0; y < s; y++) {
			// TODO: This line is causing crash.
			matrix[x][y] = false;
		}
	}
	

	int segSize = 1;
	for (int i = s / omp_get_max_threads(); i <= s; i--) {
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