#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <omp.h>
#include <mpi.h>

#define MAX_SIZE			2048
#define WIDTH(s, i, n)		((i == n-1) ? (s - i*s/n) : (s/n))
#define	MIN_X(s, i, n)		(i * s/n)

#define	NORTH	(y-1)
#define EAST	(x+1)
#define SOUTH	(y+1)
#define	WEST	(x-1)

#define WRAP(i) (((i)%s+s)%s)
#define _Y_		WRAP(y)
#define _N_		WRAP(NORTH)
#define _S_		WRAP(SOUTH)

#define N 0
#define E 1
#define S 2
#define W 3

#define YSPAN(c) (c.maxY - c.minY + 1)

#define ALLOC_ERR(pointer)	(pointer == NULL)
#define LIST_ERR(list)		((list).data == NULL)
#define PART_ERR(part)		(LIST_ERR((part).clusters) || ALLOC_ERR((part).westY))

#define BLANK_SITE		((Site){{false, false, false, false}, CLOSED})
#define BLANK_CLUSTER	((Cluster){0, 0, 0, 0})
#define BLANK_LIST		((ClusterList){NULL, 0, 0})
#define BLANK_PART		((Partition){BLANK_LIST, 0, 0, NULL, NULL})
#define BLANK_STACK 	((Stack){NULL, 0, 0})

typedef enum {SITE = 0, BOND = 1} SeedType;
typedef enum {VERT = 0, HORIZ = 1, BOTH = 2} SearchType;
typedef enum {OPEN, CLOSED} Status;

// Simple representation of a site in the lattice.
typedef struct
{
	bool bonds[4];	// Elements are set true if there is a bond in that direction.
	Status status;	// Whether a DFS has traversed this site yet.
} Site;

// Stores data relating to a cluster in the lattice or part thereof (i.e. a "subcluster").
typedef struct
{
	int size;	// Number of sites in the cluster.
	int xSpan;	// Horizontal distance covered by this cluster.
	int minY;	// Smallest (northmost) y coordinate reached by this cluster.
	int maxY; 	// Highest (southmost) y coordinate reached by this cluster.
	
} Cluster;

// Struct for storing a list of clusters and the necessary metadata.
typedef struct
{
	Cluster* data;
	int size;
	int cap;
} ClusterList;

// Stores data on what was found in a given lattice segment.
// Tempted to call it "SegmentData" to make it clear that segments and partitions
// represent different data about the same virtual structure.
typedef struct
{
	ClusterList clusters;	// List of the subclusters inside the partition.
	int minX;				// x coordinate of the partition's western border.
	int maxX;				// x coordinate of the eastern border.
	
	/*
	 *	Each element in the _Y arrays corresponds to a y coordinate on one of
	 *	the partition borders. The default value for each element is -1.
	 *	However if a subcluster is found with a bond to a site across the border,
	 *	that subcluster's index in sub is entered into the _Y element corresponding
	 *	to the point where it crosses the border.
	 *	This facilitates the merging of subclusters that make contact across borders.
	 */
	
	int* westY;		// Record of subclusters that cross the western border.
	int* eastY;		// Record of subclusters that cross the eastern border.
} Partition;

// Dynamically sized stack for use in DFS.
typedef struct
{
	int* data;
	int size;
	int cap;
} Stack;

// Structure management functions.
double*** probsAlloc(int s, unsigned int seed, SeedType t);

Site** segmentAlloc(int s, int w, int minX, double*** probs, double p, SeedType t);
void freeSegment(Site** segment);

Stack stackAlloc(int w);
void pushStack(Stack *stack, int i);
int popStack(Stack *stack);
void freeStack(Stack *stack);

ClusterList listAlloc(int s);
void addCluster(Cluster new, ClusterList* list);
void freeList(ClusterList *list);

Partition partAlloc(int s, int west, int east);
Partition copyPartition(Partition* src, int s);
void freePartition(Partition *part);

// Functions for aggregating results of the partial searches.
ClusterList mergePartitions(int s, int nParts, Partition* parts);
Partition mergeSubSegments(int s, int nSubs, Partition* subs);

// Functions for analysing a lattice segment.
int subSegmentSize(int s, int w);
Partition parallelSearch(Site** segment, int s, int w);
Partition sequentialSearch(Site** segment, int s, int w);

// Functions for testing the program.
void segmentTest();
void latticeTest();
void sequentialTest();
void parallelTest();
void mergeTests();
