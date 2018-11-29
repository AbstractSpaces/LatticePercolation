#include "project.h"

//Chuck these in whichever file
void create_mpi_cluster_type(MPI_Datatype *mpi_cluster_type)
{
	int count = 4;
	int blocklengths[4] = {1, 1, 1, 1};
	MPI_Datatype types[4] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT};

	MPI_Aint offset[4];
	offset[0] = offsetof(Cluster, size);
	offset[1] = offsetof(Cluster, xSpan);
	offset[2] = offsetof(Cluster, minY);
	offset[3] = offsetof(Cluster, maxY);

	MPI_Type_create_struct(count, blocklengths, offset, types, mpi_cluster_type);
	MPI_Type_commit(mpi_cluster_type);
}

void create_mpi_clusterlist_type(MPI_Datatype *mpi_clusterlist_type, MPI_Datatype mpi_cluster_type, int latticeSize)
{
	int count = 3;
	int blocklengths[3] = {latticeSize, 1, 1,};
	MPI_Datatype types[3] = {mpi_cluster_type, MPI_INT, MPI_INT};

	MPI_Aint offset[3];
	offset[0] = offsetof(ClusterList, data);
	offset[1] = offsetof(ClusterList, size);
	offset[2] = offsetof(ClusterList, cap);

	MPI_Type_create_struct(count, blocklengths, offset, types, mpi_clusterlist_type);
	MPI_Type_commit(mpi_clusterlist_type);
}

void create_mpi_partition_type(MPI_Datatype *mpi_partition_type, MPI_Datatype mpi_clusterlist_type, int latticeSize)
{
	int count = 5;
	int blocklengths[5] = {latticeSize, 1, 1, latticeSize, latticeSize};
	MPI_Datatype types[5] = {mpi_clusterlist_type, MPI_INT, MPI_INT, MPI_INT, MPI_INT};

	MPI_Aint offset[5];
	offset[0] = offsetof(Partition, clusters);
	offset[1] = offsetof(Partition, minX);
	offset[2] = offsetof(Partition, maxX);
	offset[3] = offsetof(Partition, eastY);
	offset[4] = offsetof(Partition, westY);

	MPI_Type_create_struct(count, blocklengths, offset, types, mpi_partition_type);
	MPI_Type_commit(mpi_partition_type);
}

int main(int argc, char *argv[])
{
	if(argc > 1)
	{
		SeedType seedType;
		double probability;
		SearchType searchType;
		int size;
		int intvar;
		
		//Check the number of arguments
		if(argc != 5)
		{
			fprintf(stderr, "Error: Incorrect number of arguments.\nUsage: mpirun --hostfile host ./exe/project [seed type] [seed probability] [percolation type] [size]\n");
			exit(EXIT_FAILURE);
		}
		
		//Check the seed type
		if(strcmp(argv[1], "b") == 0) seedType = BOND;
		else if(strcmp(argv[1], "s") == 0) seedType = SITE;
		else
		{
			fprintf(stderr, "Error: Invalid seed type. Use 'b' for BOND or 's' for SITE.\n");
			exit(EXIT_FAILURE);
		}
		
		//Check the probability
		if(atof(argv[2]) < 1 && atof(argv[2]) > 0) probability = atof(argv[2]);
		else
		{
			fprintf(stderr, "Error: Invalid probability. Enter a double between 0 and 1.\n");
			exit(EXIT_FAILURE);
		}
		
		//Check the percolation type
		if (sscanf (argv[3], "%i", &intvar) != 1)
		{
			fprintf(stderr, "Error: Invalid percolation type. Use '0' for horizontal, '1' for vertical, or '2' for both.\n");
			exit(EXIT_FAILURE);
		}
		else if(atoi(argv[3]) >= 0 && atoi(argv[3]) <= 2) searchType = atoi(argv[3]);
		else
		{
			fprintf(stderr, "Error: Invalid percolation type. Use '0' for horizontal, '1' for vertical, or '2' for both.\n");
			exit(EXIT_FAILURE);
		}
		
		//Check lattice size
		if (sscanf (argv[4], "%i", &intvar) != 1)
		{
			fprintf(stderr, "Error: Invalid lattice size. Enter a valid positive integer.\n");
			exit(EXIT_FAILURE);
		}
		else if(atoi(argv[4]) < 1)
		{
			fprintf(stderr, "Error: Invalid lattice size. Enter a valid positive integer.\n");
			exit(EXIT_FAILURE);
		}
		else size = atoi(argv[4]);

		//Initialise MPI
		int required = MPI_THREAD_FUNNELED;
		int provided;
		MPI_Init_thread(NULL, NULL, required, &provided);
		if(provided < required)
		{
			fprintf(stderr, "The required thread support was not provided\n");
			exit(EXIT_FAILURE);
		}
		int world_size, world_rank;
		MPI_Comm_size(MPI_COMM_WORLD, &world_size);
		MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

		//Send the seed for the lattice to all processors
		unsigned int seed;
		if(world_rank == 0)
		{
			seed = time(NULL);
			for(int i = 1; i < world_size; i++)
			{
				MPI_Send(&seed, 1, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD);
			}
		}
		else MPI_Recv(&seed, 1, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		
		// Run the search for this process.
		Partition result = BLANK_PART;
		double*** probs = probsAlloc(size, seed, seedType);

		if(!ALLOC_ERR(probs))
		{
			int w = WIDTH(size, world_rank, world_size);
			
			Site** segment = segmentAlloc(size, w, MIN_X(size, world_rank, world_size), probs, probability, seedType);
			
			if(!ALLOC_ERR(segment))
			{	
				result = parallelSearch(segment, size, w);
				freeSegment(segment);
			}
			free(probs);
		}

		//Initialise MPI datatypes required for sending
		MPI_Datatype mpi_cluster_type, mpi_clusterlist_type, mpi_partition_type;
		create_mpi_cluster_type(&mpi_cluster_type);
		create_mpi_clusterlist_type(&mpi_clusterlist_type, mpi_cluster_type, size);
		create_mpi_partition_type(&mpi_partition_type, mpi_clusterlist_type, size);
			
		if(world_rank > 0)
		{
			MPI_Send(&result, 1, mpi_partition_type, 0, 0, MPI_COMM_WORLD);
		}
		else
		{
			Partition results[world_size];
			results[0] = result;
			ClusterList output = BLANK_LIST;
			
			bool fail;
			if(PART_ERR(results[0])) fail = true;
			else fail = false;
			
			for(int i = 1; i < world_size; i++)
			{
				MPI_Recv(&results[i], 1, mpi_partition_type, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				if(PART_ERR(results[i])) fail = true;
			}
			
			// Merge subclusters from all the partitions.
			if(!fail) output = mergePartitions(size, world_size, results);
			
			if(LIST_ERR(output))
			{
				printf("An error occurred, exiting.\n");
				return EXIT_FAILURE;
			}
			else
			{
				// Find the size of the largest cluster and whether the lattice percolates.
				int largest = 0;
				bool vPerc = false;
				bool hPerc = false;
				
				for(int i = 0; i < output.size; i++)
				{
					if(output.data[i].size > largest) largest = output.data[i].size;
					if(output.data[i].maxY - output.data[i].minY >= size) vPerc= true;
					if(output.data[i].xSpan >= size) hPerc = true;
				}
				
				printf("Largest cluster: %d.\n", largest);
				printf("Lattice percolated ");
				
				if(searchType == VERT && vPerc) printf("vertically.\n");
				else if(searchType == HORIZ && hPerc) printf("horizontally.\n");
				else if(vPerc && hPerc) printf("In all directions.\n");
			}
		}
		MPI_Type_free(&mpi_partition_type);
		MPI_Type_free(&mpi_clusterlist_type);
		MPI_Type_free(&mpi_cluster_type);
		MPI_Finalize();
	}
	else
	{
		printf("No arguments given, entering test mode.\n");
		
//		latticeTest();
//		segmentTest();
//		sequentialTest();
		parallelTest();
//		mergeTests();
	}
	
	printf("\n");
}
