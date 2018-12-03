#include "perclib.h"

int main(int argc, char** argv) {
	if (argc == 4 && argv[1][0] == 'l') {
		printf("Testing lattice.\n");
		int s = atoi(argv[2]);
		double p = atof(argv[3]);
		struct Lattice lat = newLattice(s, p);
		printLattice(lat);
		_aligned_free(lat.xy);
		return EXIT_SUCCESS;
	}
	else if (argc == 3 && argv[1][0] == 's') {
		printf("Testing stack.\n");
		int s = atoi(argv[2]);
		struct Stack stack = newStack(s);
		printf("Testing push & reallocate.\n");
		for (int i = 0; i < s * 2; i++) {
			printf("Push #%d\n", i);
			pushStack(&stack, (struct Vertex) { i, i });
		}

		printf("Stack size: %d\n", stack.size);
		printf("Stack data:\n");
		for (int i = 0; i < stack.size; i++) {
			struct Vertex v = stack.data[i];
			printf("%d, %d\n", v.x, v.y);
		}

		printf("Testing pop:\n");
		for (int i = stack.size - 1; stack.size > 0; i--) {
			printf("Pop #%d\n", i);
			struct Vertex v = popStack(&stack);
			printf("%d, %d\n", v.x, v.y);
		}
		printf("Testing pop on empty stack.\n");
		popStack(&stack);
		printf("You shouldn't be reading this.\n");
	}
	else if (argc == 4 && argv[1][0] == 't') {
		int s = atoi(argv[2]);
		double p = atof(argv[3]);
		printf("Building Tracker.\n");
		struct Tracker tracker = newTracker(s);
		printf("Tracker built.\n");
		printf("segSize: %\n", tracker.segSize);
		srand(time(NULL));
		for (int x = 0; x < s; x++) {
			for (int y = 0; y < s; y++) {
				printf("Setting Tracker %d, %d", x, y);
				if ((double)rand() / (double)RAND_MAX < p) {
					tracker.data[x][y] = true;
				}
			}
		}
		printf("Printing Tracker.\n");
		printTracker(tracker);
		_aligned_free(tracker.data);
		exit(EXIT_SUCCESS);
	}

	printf("No valid arguments.\n");
}