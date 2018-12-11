#include "lib.h"

// Functions for finding cache line size, important for proper memory alignment.

// Used when system information about cache lines can't be retrieved.
int cache_default() {
	printf("Unable to determine cache line size. Defaulting to 64 bytes.\n");
	return 64;
}

// Obtain the size of cache lines on the host system.
size_t get_cache_line() {
	// Start by figuring how big a buffer is needed.
	DWORD buff_len = 0;
	SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* buffer = 0;

	// This call is expected to fail, if it succeeds something weird has happened.
	if (GetLogicalProcessorInformationEx(RelationCache, buffer, &buff_len)) {
		printf("Error finding buffer size for GetLogicalProcessorInformationEx(). Abort.\n");
		return cache_default();
	}

	// Now bufferLength should be set correctly, so allocate the buffer and make the call again.
	buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)malloc(buff_len);
	if (buffer == NULL) {
		printf("Buffer allocation for GetLogicalProcessorInformationEx() failed. Abort.\n");
		return cache_default();
	}

	// Get the desired information for real this time.
	if (!GetLogicalProcessorInformationEx(RelationCache, buffer, &buff_len)) {
		printf("GetLogicalProcessorInformationEx() failed. Abort.\n");
		return cache_default();
	}

	// How many entries were placed in the buffer.
	int buff_size = buff_len / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);
	size_t cache = -1;

	// Finally, find the size of the L1 cache lines.
	for (int i = 0; i < buff_size; i++) {
		if (buffer[i].Cache.Level == 1) {
			printf("Cache lines are %d bytes long.\n", buffer[i].Cache.LineSize);
			cache = buffer[i].Cache.LineSize;
			break;
		}
	}

	free(buffer);

	if (cache < 0) return cache_default();
	else return cache;
}