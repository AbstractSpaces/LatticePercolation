#include "lib.h"

int testLattice(int s, double p) {
	printf("Testing lattice.\n");

	struct Lattice lat = new_lattice(s, p);
	print_lattice(lat);

	free(lat.data);
	return EXIT_SUCCESS;
}

int testStack(int s) {
	printf("Testing stack.\n");

	struct Stack stack = new_stack(s);

	printf("Testing push & reallocate.\n");
	for (int i = 0; i < s * 2; i++) {
		printf("Push #%d\n", i);
		push_stack(&stack, (struct Vertex) { i, i });
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
		struct Vertex v = pop_stack(&stack);
		printf("%d, %d\n", v.x, v.y);
	}

	printf("Testing pop on empty stack.\n");
	pop_stack(&stack);
	printf("You shouldn't be reading this.\n");
	return EXIT_FAILURE;
}

int testTracker(int s, double p) {
	printf("Testing Tracker.\n");

	struct Tracker tracker = new_tracker(s);

	printf("Tracker built.\n");
	printf("seg_size: %d\n", tracker.seg_size);

	srand((unsigned int)time(NULL));
	for (int x = 0; x < s; x++) {
		for (int y = 0; y < s; y++) {
			if ((double)rand() / (double)RAND_MAX < p) {
				tracker.data[x][y] = true;
			}
		}
	}

	printf("Printing Tracker.\n");
	print_tracker(tracker);
	_aligned_free(tracker.data);
	return EXIT_SUCCESS;
}

int test_serial(int s, double p) {
	struct Lattice lat = new_lattice(s, p);
	print_lattice(lat);
	struct Cluster_List list = serial_dfs(lat);
	print_cluster_list(list);
	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
	if (argc == 4 && argv[1][0] == 'l') {
		int s = atoi(argv[2]);
		double p = atof(argv[3]);
		return testLattice(s, p);
	}
	else if (argc == 3 && argv[1][0] == 's') {
		int s = atoi(argv[2]);
		return testStack(s);
	}
	else if (argc == 4 && argv[1][0] == 't') {
		int s = atoi(argv[2]);
		double p = atof(argv[3]);
		return testTracker(s, p);
	}
	// strmp() isn't working as expected, hence individual character comparisons.
	else if (argc == 4 && argv[1][0] == 'd' && argv[1][1] == 's') {
		int s = atoi(argv[2]);
		double p = atof(argv[3]);
		return test_serial(s, p);
	}

	printf("No valid arguments.\n");
	return EXIT_SUCCESS;
}