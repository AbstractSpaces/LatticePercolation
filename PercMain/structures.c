#include "project.h"

#define GLOBAL_X		(x + minX)
#define GLOBAL_E		WRAP(GLOBAL_X+1)
#define GLOBAL_W		WRAP(GLOBAL_X-1)
#define PROB 			((double)rand() / (double)RAND_MAX)

#define CACHE 64

#define PROBS_ROW		(s * sizeof(double**))
#define PROBS_COLS		(s * s * sizeof(double*))
#define PROBS_DATA		(s * s * 2 * sizeof(double))

#define SEGMENT_ROW		(w * sizeof(Site*))
#define SEGMENT_COL		(s * sizeof(Site))
#define SEGMENT_DATA	(w * SEGMENT_COL)

#define STACK_DATA(cap)		(cap * sizeof(int))
#define LIST_DATA(cap)		(cap * sizeof(Cluster))
#define Y_SIZE				(2 * s * sizeof(int))

/************************** Memory freeing functions **************************/

void freeSegment(Site** segment)
{
	if(!ALLOC_ERR(segment))
	{
		if(!ALLOC_ERR(segment[0])) free(segment[0]);
		free(segment);
	}
}

void freeList(ClusterList *list)
{
	if(!LIST_ERR(*list))
	{
		free(list->data);
		*list = BLANK_LIST;
	}
}

void freePartition(Partition *part)
{
	freeList(&part->clusters);
	if(!ALLOC_ERR(part->westY)) free(part->westY);
	*part = BLANK_PART;
}

void freeStack(Stack *stack)
{
	if(!LIST_ERR(*stack))
	{
		free(stack->data);
		*stack = BLANK_STACK;
	}
}

/************************ Memory allocation functions *************************/

// Allocate and fill an array of probabilities for seeding the lattice with.
double*** probsAlloc(int s, unsigned int seed, SeedType t)
{
	srand(seed);
	
	double*** probs = (double***)malloc(PROBS_ROW + PROBS_COLS + PROBS_DATA);

	if(ALLOC_ERR(probs))
	{
		printf("Probs allocation failure.\n");
		return NULL;
	}
	
	// The first s elements are pointers to the columns.
	probs[0] = (double**)probs + s;
	// The next s*s elements are pointers to the arrays of data.
	probs[0][0] = (double*)probs[0] + s*s;
	
	for(int x = 0; x < s; x++)
	{
		probs[x] = (double**)probs[0] + x*s;
		
		for(int y = 0; y < s; y++)
		{
			probs[x][y] = (double*)probs[0][0] + x*s*2 + y*2;
			
			if(t == SITE)
			{
				probs[x][y][SITE] = PROB;
			}
			else
			{
				// The southern bond is equivalent to the southern
				// neighbour's north bond, hence we don't need to store
				// southern bonds.
				probs[x][y][N] = PROB;
				// Likewise for west and east.
				probs[x][y][E] = PROB;
			}
		}
	}

	return probs;
}

// Each process will allocate the lattice segment it is assigned, and
// then subdivide work on it among its threads.
Site** segmentAlloc(int s, int w, int minX, double*** probs, double p, SeedType t)
{	
	if(ALLOC_ERR(probs)) return NULL;
	
	Site** segment = (Site**)malloc(SEGMENT_ROW);

	if(ALLOC_ERR(segment))
	{
		printf("Lattice segment row allocation failure.\n");
		return NULL;
	}

	Site* segCols;
	
	if(posix_memalign((void**)&segCols, CACHE, SEGMENT_DATA) != 0)
	{
		printf("Lattice segment col allocation failure.\n");
		segCols = NULL;
		freeSegment(segment);
		return NULL;
	}
	
	#pragma omp parallel default(shared)
	{
		#pragma omp for schedule(static)
		for(int x = 0; x < w; x++)
		{
			segment[x] = segCols + x*s;
			
			for(int y = 0; y < s; y++)
			{
				// Initialise sites to default values.
				segment[x][y] = BLANK_SITE;
				// Sites are assumed to be occupied with bond seeding.
				if(t == BOND) segment[x][y].status = OPEN;
			}
		}
	
		// Examine which sites are connected to which others.
		#pragma omp for schedule(static)
		for(int x = 0; x < w; x++)
		{
			for(int y = 0; y < s; y++)
			{
				// If this site is occupied, check each of the neighbours and set the bonds accordingly.
				if(t == SITE && probs[GLOBAL_X][y][SITE] <= p)
				{
					segment[x][y].status = OPEN;
					if(probs[GLOBAL_X][_N_][SITE] <= p) segment[x][y].bonds[N] = true;
					if(probs[GLOBAL_E][y][SITE] <= p) segment[x][y].bonds[E] = true;
					if(probs[GLOBAL_X][_S_][SITE] <= p) segment[x][y].bonds[S] = true;
					if(probs[GLOBAL_W][y][SITE] <= p) segment[x][y].bonds[W] = true;
				}
				else if(t == BOND)
				{
					if(probs[GLOBAL_X][y][N] <= p) segment[x][y].bonds[N] = true;
					if(probs[GLOBAL_X][y][E] <= p) segment[x][y].bonds[E] = true;
					if(probs[GLOBAL_X][_S_][N] <= p) segment[x][y].bonds[S] = true;
					if(probs[GLOBAL_W][y][E] <= p) segment[x][y].bonds[W] = true;
				}
			}
		}
	}
	return segment;	
}

// Each thread will need to allocate a private stack for use in DFS.
Stack stackAlloc(int s)
{
	Stack stack = BLANK_STACK;
	stack.cap = s;
	
	// Aligning with cache lines avoids threads writing to and invalidating
	// each other's lines.
	if(posix_memalign((void**)&stack.data, CACHE, STACK_DATA(stack.cap)) != 0)
	{
		printf("Stack allocation failure.\n");
		return BLANK_STACK;
	}
	else return stack;
}

ClusterList listAlloc(int s)
{
	ClusterList list = BLANK_LIST;
	list.cap = s;
	
	// Threads will be working in parallel on subsegments (i.e. partitions), which
	// contain cluster lists and hence cache alignment is desirable.
	if(posix_memalign((void**)&list.data, CACHE, LIST_DATA(list.cap)))
	{
		printf("List allocation failure.\n");
		return BLANK_LIST;
	}
	else return list;
}

Partition partAlloc(int s, int west, int east)
{
	Partition part = BLANK_PART;
	part.minX = west;
	part.maxX = east;
	part.clusters = listAlloc(s);
	
	if(LIST_ERR(part.clusters))
	{
		printf("Aborting partition allocation.\n");
		return BLANK_PART;
	}

	if(posix_memalign((void**)&part.westY, CACHE, Y_SIZE) != 0)
	{
		printf("Border record allocation failure.\n");
		freeList(&part.clusters);
		return BLANK_PART;
	}
	else
	{
		part.eastY = (int*)part.westY + s;
		memset(part.westY, -1, s * sizeof(int));
		memset(part.eastY, -1, s * sizeof(int));
		return part;
	}
}

Partition copyPartition(Partition* src, int s)
{
	Partition dest = BLANK_PART;
	
	dest.minX = src->minX;
	dest.maxX = src->maxX;
	dest.clusters = listAlloc(src->clusters.cap);
	dest.clusters.cap = src->clusters.cap;
	dest.clusters.size = src->clusters.size;
	
	if(LIST_ERR(dest.clusters))
	{
		printf("\nAllocating space for cluster list copy failed.");
		freePartition(&dest);
		return BLANK_PART;
	}
	else memcpy(dest.clusters.data, src->clusters.data, dest.clusters.size * sizeof(Cluster));
	
	if(posix_memalign((void**)&dest.westY, CACHE, Y_SIZE) != 0)
	{
		printf("\nAllocating space for border list copy failed.");
		freePartition(&dest);
		return BLANK_PART;
	}
	else
	{
		memcpy(dest.westY, src->westY, Y_SIZE);
		dest.eastY = &dest.westY[s];
	}
	
	return dest;
}

/*********************** Functions for using structures ***********************/

// Push a new cluster to a list of clusters.
void addCluster(Cluster new, ClusterList* list)
{
	if(!LIST_ERR(*list))
	{
		// Grow the list.
		if(list->size >= list->cap)
		{
			Cluster* old = list->data;
			list->cap *= 2;
			
			if(posix_memalign((void**)&list->data, CACHE, LIST_DATA(list->cap)) != 0)
			{
				printf("Growing cluster list failed.\n");
				free(old);
				freeList(list);
				return;
			}
			else
			{
				memcpy(list->data, old, list->size * sizeof(Cluster));
				free(old);
			}
		}

		list->data[list->size] = new;
		list->size += 1;
	}
}

void pushStack(Stack *stack, int i)
{	
	if(!LIST_ERR(*stack))
	{
		// The stack may need to be grown dynamically.
		if(stack->size >= stack->cap)
		{
			int* old = stack->data;
			stack->cap *= 2;
			
			if(posix_memalign((void**)&stack->data, CACHE, STACK_DATA(stack->cap)) != 0)
			{
				printf("Growing stack failed.\n");
				free(old);
				freeStack(stack);
				return;
			}
			else
			{
				memcpy(stack->data, old, stack->size * sizeof(int));
				free(old);
			}
		}
		
		stack->data[stack->size] = i;
		stack->size += 1;
	}
}

int popStack(Stack *stack)
{
	if(stack->size > 0)
	{
		stack->size -= 1;
		return stack->data[stack->size];
	}
	// Nothing to pop.
	else return -1;
}

// Find a cache-aligned number of lattice columns that each thread should be
// assigned.
int subSegmentSize(int w, int s)
{
	// More chunks than threads allows some degree of dynamic scheduling, hopefully
	// preventing thread starvation.
	int subS = w / (omp_get_max_threads()+1);
	// Ajust the number of columns per chunk until they are cache aligned.
	while((subS * SEGMENT_COL) % CACHE != 0 && subS > 1) subS--;
	return subS;
}
