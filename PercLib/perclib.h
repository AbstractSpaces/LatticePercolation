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
	// The actual 2D matrix.
	struct Site** data;
};

// Rather than modify the lattice, search algorithms use Trackers to remember which Sites have already been traversed.
struct Tracker {
	int size;
	// Number of columns in a segment.
	int segSize;
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

size_t getCacheLine();

struct Lattice newLattice(int s, double p);
void printLattice(struct Lattice lat);

struct Tracker newTracker(int s);
void printTracker(struct Tracker tracker);

struct Stack newStack(int s);
void pushStack(struct Stack* stack, struct Vertex new);
struct Vertex popStack(struct Stack* stack);