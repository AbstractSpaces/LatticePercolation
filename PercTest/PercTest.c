#include "percmain.h"

int main(int argc, char** argv) {
	if (argc > 1) {
		if (argv[1][0] == 'l') {
			printf("Testing lattice.\n");
			int s = atoi(argv[2]);
			double p = atof(argv[3]);
			struct Lattice lat = newLattice(s, p);
			printLattice(lat);
			_aligned_free(lat.xy);
			return EXIT_SUCCESS;
		}
	}
	
	printf("No valid arguments.\n");
}