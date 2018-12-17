#pragma once

// Frankly I'm unsure of the best way to structure includes for a project, but this project is small enough that I can throw everything into one header.

#include "stdafx.h"

// A Lattice is a 2D matrix of Sites.
struct Site {
	// Each bool represents a bond in a direction away from the site.
	// In terms of array indices where the array is accessed with A[x][y], the directions are:
	// up == >y, down == <y, left == >x, right == <x.
	bool up, down, left, right;
};

struct Lattice {
	// The length of one side of the lattice.
	int size;
	// The actual 2D matrix.
	struct Site** data;
};

// Rather than modify the lattice, search algorithms use Trackers to remember which Sites have already been traversed.
struct Tracker {
	int size;
	// Number of columns in a segment.
	int seg_size;
	// True if a site has been traversed.
	bool** data;
};

// It's less hassle to store co-ordinates in a 1 dimensional array of structs instead of a 2D array.
struct Vertex {
	int x;
	int y;
};

// Iterative DFS algorithms require a stack.
struct Stack {
	// Number of items currently on the stack.
	int size;
	// Macimum size with currently allocated memory.
	int max;

	struct Vertex* data;
};

struct Cluster {
	// Clusters need unique identifiers to prevent the same pair being stitched twice.
	int id;
	// Number of sites contained.
	int size;
	// Dimensions indicating how far the cluster spans.
	int x_min, x_max, y_min, y_max;
};

struct Cluster_List {
	int size;
	int max;
	struct Cluster* data;
};

size_t get_cache_line();

struct Lattice new_lattice(int s, double p);
bool check_empty(struct Site s);
void print_lattice(struct Lattice lat);

struct Tracker new_tracker(int s);
void print_tracker(struct Tracker tracker);

struct Stack new_stack(int s);
void push_stack(struct Stack* stack, struct Vertex new);
struct Vertex pop_stack(struct Stack* stack);

struct Cluster stitch_clusters(struct Cluster left, struct Cluster right, int* num_clusters);
struct Cluster_List new_cluster_list(int s);
void push_cluster(struct Cluster_List* list, struct Cluster new);