#include "percmain.h"

// Functions for creating and managing lattice structures.

// Get a probability between 0 and 1.
double prob() {
	return (double)rand() / (double)RAND_MAX;
}

// Create a lattice with size s and bond probability p.
struct Lattice newLattice(int s, double p) {
	// To prevent false sharing, columns in different segments shouldn't appear on the same cache line.
	// So the allocation process starts by finding how cache lines are aligned.
	size_t cache = getCacheLine();

	// This method of allocating a 2D array was the main thing I learned from this project.
	// The whole thing occupies a single contiguous chunk of memory, requiring only one malloc() and one free().
	// The block starts with a header of double pointers, each pointing to the beginning of a column containing actual data.
	size_t header = s * sizeof(struct Site**);
	
	// To ensure the first column is cache aligned, there needs to be a gap of empty memory between it and the header.
	size_t offset = header % cache;

	// Finally, after the offset comes the memory actually holding Site data.
	size_t data = s * s * sizeof(struct Site);

	// Now we allocate the memory for this pointer Frankenstein.
	struct Site** matrix = (struct Site**)_aligned_malloc(header + offset + data, cache);

	if (matrix == NULL) {
		printf("Lattice allocation failed. Abort.\n");
		// Is it dirty and cheap to just exit with no real handling or cleanup? Yes.
		// Do I currently have the time to do otherwise? Nope.
		exit(EXIT_FAILURE);
	}
	
	// Now for the magic part. We take advantage of the fact that for double pointer m: m[x][y] is equivalent to *(*m + x) + y.
	// We just make sure *m + x points to the appropriate place in the data area.
	// First, get the adress of the first element in the data area.
	struct Site* first = (struct Site*)((char*)matrix + header + offset);

	for (int x = 0; x < s; x++) {
		matrix[x] = first + x * s;
		
		// Now initialise the Sites along column x.
		for (int y = 0; y < s; y++) {
			matrix[x][y] = (struct Site){ false, false, false, false };
		}
	}

	// With all sites initialised, now set the bonds.
	srand((unsigned int)time(NULL));

	for (int x = 0; x < s; x++) {
		for (int y = 0; y < s; y++) {
			if ((x < s - 1) && (prob() < p)) {
				matrix[x][y].right = true;
				matrix[x + 1][y].left = true;
			}

			if ((y < s - 1) && (prob() < p)) {
				matrix[x][y].down = true;
				matrix[x][y+1].up = true;
			}
		}
	}

	// Finally, find how many columns will form a cache aligned segment.
	// Start with 1 segment per available processor and decrement until aligned.
	int segSize = s;
	for (int i = s / omp_get_max_threads(); i <= s; i--) {
		if (i * s * sizeof(struct Site) % cache == 0) {
			segSize = i;
			break;
		}
	}

	return (struct Lattice){s, segSize, matrix};
}

// Check if a lattice site has any bonds and should be considered empty.
bool checkEmpty(struct Site s) {
	return !(s.down || s.up || s.left || s.right);
}

// Print a lattice to file.
void printLattice(struct Lattice lat) {
	FILE* out;

	if (fopen_s(&out, "lattice.txt", "w") != 0) {
		printf("Unable to open file for printing.\n");
		return;
	}

	fprintf(out, "Lattice size: %d\nSegment size: %d\n\n", lat.size, lat.segSize);

	for (int y = 0; y < lat.size; y++) {
		for (int x = 0; x < lat.size; x++) {
			if (checkEmpty(lat.xy[x][y])) {
				fprintf(out, "  ");
			}
			else {
				fprintf(out, "O");
				if (lat.xy[x][y].right) {
					fprintf(out, "-");
				}
				else {
					fprintf(out, " ");
				}
			}
		}

		fprintf(out, "\n");

		for (int x = 0; x < lat.size; x++) {
			if (lat.xy[x][y].down) {
				fprintf(out, "| ");
			}
			else {
				fprintf(out, "  ");
			}
		}

		fprintf(out, "\n");
	}

	fclose(out);
}