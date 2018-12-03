#pragma once

// Frankly I'm unsure of the best way to structure includes for a project, but this project is small enough that I can throw everything into one header.

#include "stdafx.h"

// A Lattice is a 2D matrix of Sites.
struct Site {
	// Each bool represents a bond in a direction away from the site.
	bool up, down, left, right;
};

struct Lattice {
	// The length of one side of the lattice.
	int size;
	// Number of columns in a segment.
	int segSize;
	// The actual 2D matrix.
	struct Site** xy;
};

// Rather than modify the lattice, search algorithms use Trackers to remember which Sites have already been traversed.
// Like Stacks, threads will have their own Tracker.
// Hence it needs to be cache aligned, but also it won't be square and needs a width specified.
struct Tracker {
	int width;
	int height;
	bool** done;
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

size_t getCacheLine();

struct Lattice newLattice(int s, double p);
void printLattice(struct Lattice lat);

struct Tracker newTracker(int w, int s);

struct Stack newStack(int s);
void pushStack(struct Stack* stack, struct Vertex new);
struct Vertex popStack(struct Stack* stack);