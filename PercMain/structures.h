#pragma once

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
struct Tracker {
	bool** done;
};