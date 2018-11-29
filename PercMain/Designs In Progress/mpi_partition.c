#include <mpi.h>
#include <stdio.h>
#include <omp.h>

void create_mpi_cluster_type(MPI_Datatype *mpi_cluster_type)
{
	int count = 5;
	int blocklengths[5] = {1, 1, 1, 1, 1};
	MPI_Datatype types[5] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};

	MPI_Aint offset[5];
	offset[0] = offsetof(Cluster, size);
	offset[1] = offsetof(Cluster, minY);
	offset[2] = offsetof(Cluster, minX);
	offset[3] = offsetof(Cluster, maxY);
	offset[4] = offsetof(Cluster, maxX);


	MPI_Type_create_struct(count, blocklengths, offset, types, mpi_cluster_type);
	MPI_Type_commit(mpi_cluster_type);
}

void create_mpi_partition_type(MPI_Datatype *mpi_partition_type, MPI_Datatype mpi_cluster_type)
{
	int count = 6;
	int blocklengths[6] = {1, latticeSize, 1, 1, latticeSize, latticeSize};
	MPI_Datatype types[6] = {MPI_INT, mpi_cluster_type, MPI_INT, MPI_INT, MPI_INT, MPI_INT};

	MPI_Aint offset[6];
	offset[0] = offsetof(Partition, nSubs);
	offset[1] = offsetof(Partition, sub);
	offset[2] = offsetof(Partition, minX);
	offset[3] = offsetof(Partition, maxX);
	offset[4] = offsetof(Partition, eastY);
	offset[5] = offsetof(Partition, westY);

	MPI_Type_create_struct(count, blocklengths, offset, types, mpi_partition_type);
	MPI_Type_commit(mpi_partition_type);
}

int main(int argc, char** argv) {
	int required = MPI_THREAD_FUNNELED;
	int provided;
	MPI_Init_thread(NULL, NULL, required, &provided);
	int world_size, world_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	int name_len;
	MPI_Get_processor_name(processor_name, &name_len);

	//Send the seed for the lattice
	unsigned int seed;
	if(world_rank == 0)
	{
		seed = time(NULL);
		for(int i = 1; i < world_size; i++)
		{
			MPI_Send(&seed, 1, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD);
		}
	}
	else
	{
		MPI_Recv(&seed, 1, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}

	//Allocate memory and seed the partition of lattice relevant to the processor
	//Each processor has a lattice sized nChunks by SIZE
	partitionAlloc(latticeSize, world_rank*chunk, (world_rank+1)*chunk, p, t, seed)

	//RUN DFS FOR THREADS

	//Merge the thread partitions together
	Partition partition = mergeLocalPartitions();
	Partition *partitions;

	//Once it is done, send partitions back to master process
	//Need to make MPI_type for partition before sending over MPI
	MPI_Datatype mpi_cluster_type, mpi_partition_type;
	create_mpi_cluster_type(&mpi_cluster_type); //Used within partition_type
	create_mpi_partition_type(&mpi_partition_type, mpi_cluster_type);

	if(world_rank > 0)
	{
		MPI_Send(&partition, 1, mpi_partition_type, world_rank, 0, MPI_COMM_WORLD);
	}
	else
	{
		partitions[0] = partition;
		for(int i = 1; i < world_size; i++)
		{
			partitions[i] = MPI_Recv(&partition, 1, mpi_partition_type, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
	}

	//Master receives and stitches up the processor partitions
	int nParts = sizeof(partitions) / sizeof(partitions[0]);
	Cluster *cluster = mergePartitions(int s, partitions, nParts)

	MPI_Finalize();
}