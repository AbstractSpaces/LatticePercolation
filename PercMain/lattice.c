#include "percmain.h"

// Used when system information about cache lines can't be retrieved.
int cacheDefault() {
	printf("Unable to determine cache line size. Defaulting to 64 bytes.\n");
	return 64;
}

// Obtain the size of cache lines on the host system.
size_t getCacheSize() {
	// Start by figuring how big a buffer is needed.
	DWORD bufferLength = 0;
	SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* buffer = 0;

	// This call is expected to fail, if it succeeds something weird has happened.
	if (GetLogicalProcessorInformationEx(RelationCache, buffer, &bufferLength)) {
		printf("Error finding buffer size for GetLogicalProcessorInformationEx(). Abort.\n");
		return cacheDefault();
	}

	// Now bufferLength should be set correctly, so allocate the buffer and make the call again.
	buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)malloc(bufferLength);
	if (buffer == NULL) {
		printf("Buffer allocation for GetLogicalProcessorInformationEx() failed. Abort.\n");
		return cacheDefault();
	}

	// Get the desired information for real this time.
	if (!GetLogicalProcessorInformationEx(RelationCache, buffer, &bufferLength)) {
		printf("GetLogicalProcessorInformationEx() failed. Abort.\n");
		return cacheDefault();
	}

	// How many entries were placed in the buffer.
	int numReturned = bufferLength / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);
	size_t cache = -1;

	// Finally, find the size of the L1 cache lines.
	for (int i = 0; i < numReturned; i++) {
		if (buffer[i].Cache.Level == 1) {
			printf("Cache lines are %d bytes long.\n", buffer[i].Cache.LineSize);
			cache = buffer[i].Cache.LineSize;
			break;
		}
	}

	free(buffer);
	
	if (cache < 0) return cacheDefault();
	else return cache;
}

// Create a lattice with size s and bond probability p.
struct Lattice newLattice(int s, double p) {
	// To prevent false sharing, columns in different segments shouldn't appear on the same cache line.
	// So the allocation process starts by finding how cache lines are aligned.
	size_t cache = getCacheSize();

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
		exit(EXIT_FAILURE);
	}
	
	// Now for the magic part. We take advantage of the fact that for double pointer m:
	// m[x][y] is equivalent to *(*m + x) + y.
	// We just make sure *m + x points to the appropriate place in the data area.
	// First, get the adress of the first element in the data area.
	struct Site* first = (struct Site*)((char*)matrix + header + offset);

	for (int x = 0; x < s; x++) {
		matrix[x] = first + x * s;
	}

	// Finally, find how many columns will form a cache aligned segment.
	int segSize = s;
	for (int i = 1; i <= s; i++) {
		if (i * s * sizeof(struct Site) % cache == 0) {
			segSize = i;
			break;
		}
	}

	// Not sure why I couldn't do this on one line.
	struct Lattice l = {s, segSize, matrix};
	return l;
}