#include "percmain.h"

// Functions for finding cache line size, important for proper memory alignment.

// Used when system information about cache lines can't be retrieved.
int cacheDefault() {
	printf("Unable to determine cache line size. Defaulting to 64 bytes.\n");
	return 64;
}

// Obtain the size of cache lines on the host system.
size_t getCacheLine() {
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