#include "lib.h"

// Functions for creating and managing lattice structures.

// Get a probability between 0 and 1.
double prob() {
	return (double)rand() / (double)RAND_MAX;
}

// Create a lattice with size s and bond probability p.
// The allocation method is the same as for Trackers, except without the need for cache alignment.
struct Lattice new_lattice(int s, double p) {
	size_t header = s * sizeof(struct Site*);
	size_t data = s * s * sizeof(struct Site);

	struct Site** matrix = (struct Site**)malloc(header + data);

	if (matrix == NULL) {
		printf("Lattice allocation failed. Abort.\n");
		exit(EXIT_FAILURE);
	}
	
	struct Site* first = (struct Site*)((char*)matrix + header);

	for (int x = 0; x < s; x++) {
		matrix[x] = first + x * s;
		
		// Now initialise the Sites along column x.
		for (int y = 0; y < s; y++) {
			matrix[x][y] = (struct Site){ false, false, false, false };
		}
	}

	// With all sites initialised, now set the bonds.
	// "Up" and "left" bonds mirror "down" and "right" bonds respectively, so they are set at the same as their counterparts.
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

	return (struct Lattice){s, matrix};
}

// Check if a lattice site has any bonds and should be considered empty.
bool check_empty(struct Site s) {
	return !(s.down || s.up || s.left || s.right);
}

// Print a lattice to file.
void print_lattice(struct Lattice lat) {
	FILE* out;

	if (fopen_s(&out, "lattice.txt", "w") != 0) {
		printf("Unable to open file for printing.\n");
		return;
	}

	fprintf(out, "Lattice size: %d\n\n", lat.size);

	for (int y = 0; y < lat.size; y++) {
		for (int x = 0; x < lat.size; x++) {
			if (check_empty(lat.data[x][y])) {
				fprintf(out, "  ");
			}
			else {
				fprintf(out, "O");
				if (lat.data[x][y].right) {
					fprintf(out, "-");
				}
				else {
					fprintf(out, " ");
				}
			}
		}

		fprintf(out, "\n");

		for (int x = 0; x < lat.size; x++) {
			if (lat.data[x][y].down) {
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