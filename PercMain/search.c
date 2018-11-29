#include "project.h"

#define SUB 		(subSegs[i])
#define MAX_X		((i == n-1) ? (w-1) : ((i+1) * subW - 1))
#define NEW_CLUSTER	((Cluster){0, 1, y, y})

void dfs(Site** segment, Partition* subSeg, Stack* stack, int s, int x, int y)
{
	if(PART_ERR(*subSeg)) printf("DFS passed faulty subsegment, aborting.\n");
	else
	{
		Cluster this = NEW_CLUSTER;
		int id = subSeg->clusters.size;
		int minX = x;
		int maxX = x;

		segment[x][y].status = CLOSED;
		pushStack(stack, x);
		pushStack(stack, y);
		
		while(!LIST_ERR(*stack) && stack->size > 0)
		{
			y = popStack(stack);
			x = popStack(stack);
			this.size += 1;
			
			if(x < minX)
			{
				minX = x;
				this.xSpan += 1;
			}
			else if(x > maxX)
			{
				maxX = x;
				this.xSpan += 1;
			}

			if(y < this.minY) this.minY = y;
			else if(y > this.maxY) this.maxY = y;
			
			if(segment[x][_Y_].bonds[N] && segment[x][_N_].status == OPEN)
			{
				segment[x][_N_].status = CLOSED;
				pushStack(stack, x);
				pushStack(stack, NORTH);
			}
			
			if(segment[x][_Y_].bonds[E])
			{
				if(x == subSeg->maxX)
				{
					subSeg->eastY[_Y_] = id;
				}
				else if(segment[EAST][_Y_].status == OPEN)
				{
					segment[EAST][_Y_].status = CLOSED;
					pushStack(stack, EAST);
					pushStack(stack, y);
				}
			}
			
			if(segment[x][_Y_].bonds[S] && segment[x][_S_].status == OPEN)
			{
				segment[x][_S_].status = CLOSED;
				pushStack(stack, x);
				pushStack(stack, SOUTH);
			}
			
			if(segment[x][_Y_].bonds[W])
			{
				if(x == subSeg->minX)
				{
					subSeg->westY[_Y_] = id;
				}
				else if(segment[WEST][_Y_].status == OPEN)
				{
					segment[WEST][_Y_].status = CLOSED;
					pushStack(stack, WEST);
					pushStack(stack, y);
				}
			}
		}
		
		// Ensure minY and maxY are always measured as positive distance from 0.
		if(this.minY < 0)
		{
			int yDist = this.maxY - this.minY;
			this.minY = WRAP(this.minY);
			this.maxY = this.minY + yDist;
		}
		
		addCluster(this, &(subSeg->clusters));
	}
}

// Take a generated lattice segment, allocate threads to search parts of it.
Partition parallelSearch(Site** segment, int s, int w)
{
	if(ALLOC_ERR(segment))
	{
		printf("Search received faulty segment, aborting.\n");
		return BLANK_PART;
	}
	
	int subW = subSegmentSize(s, w);
	int n = w / subW;
	if(w % subW != 0) n++;
	
	Partition subSegs[n];
	bool fail = false;
	
	// Create team, search each subsegment.
	#pragma omp parallel default(shared)
	{
		// Allocate a stack for each thread to use.
		Stack stack = stackAlloc(w);
		bool abort = false;
		
		if(LIST_ERR(stack))
		{
			printf("Stack allocation for thread %d failed.\n", omp_get_thread_num());
			abort = true;
		}
		
		// Tell threads to search subsegments in parallel.
		#pragma omp for schedule(guided)
		for(int i = 0; i < n; i++)
		{
			SUB = partAlloc(s, i*subW, MAX_X);
			
			if(PART_ERR(SUB))
			{
				printf("SubSeg %d initialisation failure.\n", i);
				abort = true;
			}
			
			// Iterate through open sites in the subsegment.
			for(int x = SUB.minX; x <= SUB.maxX && !abort; x++)
			{	
				for(int y = 0; y < s && !abort; y++)
				{
					if(segment[x][y].status == OPEN)
					{	
						dfs(segment, &SUB, &stack, s, x, y);
						
						if(LIST_ERR(stack) || PART_ERR(SUB))
						{
							printf("Subsegment search failure in thread %d.\n", omp_get_thread_num());
							abort = true;
						}
					}
				}
			}
		}
		
		freeStack(&stack);

		if(abort)
		{
			#pragma omp atomic write
			fail = true;
			
			printf("\nThread %d aborting.\n", omp_get_thread_num());
		}
	}
	
	Partition send = BLANK_PART;
	
	if(!fail) send = mergeSubSegments(s, n, subSegs);
	for(int i = 0; i < n; i++) freePartition(&SUB);
	
	return send;
}

// Search a lattice segment in simple sequential fashion.
Partition sequentialSearch(Site** segment, int s, int w)
{
	if(ALLOC_ERR(segment))
	{
		printf("Search received faulty segment, aborting.\n");
		return BLANK_PART;
	}
	
	bool abort = false;
	
	Partition result = partAlloc(s, 0, w-1);

	if(PART_ERR(result))
	{
		printf("Partition allocation failure.\n");
		abort = true;
	}

	Stack stack = stackAlloc(w);
	
	if(LIST_ERR(stack))
	{
		printf("Stack allocation failure.\n");
		abort = true;
	}
	
	for(int y = 0; y < s && !abort; y++)
	{
		for(int x = result.minX; x <= result.maxX && !abort; x++)
		{
			if(segment[x][y].status == OPEN)
			{
				dfs(segment, &result, &stack, s, x, y);

				if(LIST_ERR(stack) || PART_ERR(result))
				{
					printf("DFS failure.\n");
					abort = true;
				}
			}
		}
	}
	
	freeStack(&stack);
	if(abort) freePartition(&result);
	
	return result;
}
