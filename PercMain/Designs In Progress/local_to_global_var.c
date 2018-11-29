#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/*
	Offset the local index of each subCluster based on the world_rank.
	The index is based on the world_rank+maxClusters for each partition.
	e.g. if the cluster rank is 6, and the max number of clusters is 50,
	the range of global indexes assigned would be 600 to 699.

	The global index can be obtained by summing globalIndexOffset and localIndex
*/

int main(int argc, char const *argv[])
{
	int world_rank = 6;
	int nCol = 5; //maxX - minX in a partition
	int latticeSize = 10; //maxY - minY in a partition
	int maxClusters = nCol*latticeSize; //Max number of clusters possible in a partition

	int nDigits = floor(log10(abs(maxClusters))) + 1; //length of maxClusters
	int globalIndexOffset = world_rank * round(pow(10, nDigits));
	printf("%i\n", globalIndexOffset); //Starting index for each partition
	return globalIndexOffset;
}