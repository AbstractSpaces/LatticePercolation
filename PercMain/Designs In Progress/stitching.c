/*
		Previous stitching method doesn't work if a cluster starts in the middle and
		wraps around. All clusterPart previously found, even beyond the neighbours,
		would have to be synchronised. Also for clusters with full wrap arounds (like a cylinder),
		it would end up summing the start column twice.

		May have to use DFS on the reduced size clusterParts
	*/

//Master thread, should be the same length as the number of threads, contains clusterParts
clusterPart clusterColumns [nChunks];

/*
	Each thread has its own, contains the clusters a thread comes across in the assigned chunk.
	Max size of latticeSize if all clusters are size 1
*/
clusterPart clusterParts [latticeSize];

typedef struct
{

	//Stitching DFS search to avoid repeated summations
	bool visited;

	//Size of the cluster part
	int clusterSize;

	/*
		Left to right, contains 0s or 1s, if sum(colSpan) == nColumns, it percolates.
		this can be summed when stitching the parts
	*/
	int colSpan[nColumns];

	/*
		Top to bottom, contains 0s or 1s, when stitching, need to check which rows contains
		a 1, then merge the results, if(rowSpan) == latticeSize, it percolates
	*/
	int rowSpan[nRows];

	/*
		Array of boundary y coordinates that connects with nodes to the left.
		Can contain a range of numbers from 0 to latticeSize-1
	*/
	int westBoundary [latticeSize];

	/*
		Array of boundary y coordinates that connects with nodes to the right.
		Can contain a range of numbers from 0 to latticeSize-1
	*/
	int eastBoundary [latticeSize];
} clusterPart;

void clusterSearch(clusterPart clusterColumns[latticeSize])
{
	clusterPart clusterColumns [world_size*nThread];

	//Only the master thread makes MPI calls
	int required = MPI_THREAD_FUNNELED;
	int provided;
	int MPI_Init_thread(NULL, NULL, required, &provided);

	int  world_rank, world_ size,  ierr;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	//Master node
	if(world_rank == 0)
	{
		for(int i = 1; i < world_size; i++)
		{
			/*
				Send chunks to other nodes, each node should have nThread number of chunks.
				Distribution of extra chunks need to be considered.
			*/
			MPI_SEND();
		}
	}
	else
	{
		for(int i = 1; i < world_size; i++)
		{
			/*
				Receive nThread number of chunks for each node
			*/
			MPI_RECV();
		}
	}
	
	//All nodes
	#pragma omp parallel
	{
		clusterPart clusterParts [latticeSize]; //Each thread has this array for each chunk they have
		
		//For < nChunks
		//For each node, check OPEN and run DFS
		clusterPart cluster = dfsSearch(); //Run DFS for each new cluster encountered
	}

	/*
		Order the clusterParts arrays for each thread and send it back to the main node
	*/
	if(world_rank > 0)
	{
		for(int i = 1; i < world_size; i++)
		{
			/*
				Send the clusterParts arrays back to the master thread
			*/
			MPI_SEND();
		}
	}
	else
	{
		for(int i = 1; i < world_size; i++)
		{
			/*
				Receive clusterParts arrays from the other nodes and store in main array
			*/
			clusterColumns[i] = MPI_RECV();
		}
	}
}

void stitch(clusterPart clusterColumns[latticeSize])
{
	for(int i = 0; i < latticeSize; i++) //Iterate through clusterParts from each thread
	{
		for(int j = 0; j < size(clusterColumns[i]); j++) //Iterate through each clusterPart
		{
			clusterPart *currentCluster = clusterColumns[i].clusterParts[j];
			if(!currentCluster.visited)
			{
				//Memset to 0 each time
				int cluster = 0;
				int colSpan [latticeSize];
				int rowSpan [latticeSize];
				cluster = dfsStitch(currentCluster, clusterColumns, i, j, colSpan, rowSpan);

				if(cluster > largest) largest = cluster;
				if(sum(colSpan) > largestCol) largestCol = sum(colSpan);
				if(sum(rowSpan) > largestRow) largestRow = sum(rowSpan);
			}
		}
	}
}

int dfsStitch(clusterPart currentCluster, clusterPart clusterColumns[latticeSize], int i, int j, int colSpan, int rowSpan)
{
	//Need the stack

	currentCluster.visited = TRUE;
	for(int x = 0; x < size(colSpan); x++) //Check the number of columns the cluster part spans
	{
		if(colSpan[x] == 0 && currentCluster.colSpan[x] == 1) colSpan[x] = 1;
	}
	for(int y = 0; y < size(rowSpan); y++) //Check the number of rows the cluster part spans
	{
		if(rowSpan[y] == 0 && currentCluster.rowSpan[y] == 1) rowSpan[y] = 1;
	}

	for(int a = 0; a < size(currentCluster.eastBoundary); a++) //Iterate through the east connections for each clusterPart
	{
		bool connected = FALSE; //Check if the nodes become linked
		for(int b = 0; b < size(clusterColumns[i+1]); b++) //Iterate through each clusterPart to the east
		{
			clusterPart *eastCluster = clusterColumns[i+1].clusterParts[b];
			if(!eastCluster.visited)
			{
				for(int c = 0; c < size(eastCluster.westBoundary); c++) //Iterate through the west connections for each clusterPart to the east
				{
					if(currentCluster.eastBoundary[a] == eastCluster.westBoundary[c]) //Check if the two clusterPart are connected
					{
						//Push to stack
						//Move on and check the next eastCluster
						connected = TRUE;
						break;
					}
				}
			}
			if(connected == TRUE) break;
		}
	}

	//REPEAT THIS FOR WEST BOUNDARY CONNECTIONS
}