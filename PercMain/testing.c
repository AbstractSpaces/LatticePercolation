#include "project.h"

#define HI	0
#define MID	1
#define LO	2

void printRow(Site** seg, int y, int s, int w, int lvl)
{
	if(lvl == HI)
	{
		for(int x = 0; x < w; x++)
		{
			if(seg[x][y].bonds[N]) printf(" | ");
			else printf("   ");
		}
	}
	else if(lvl == MID)
	{
		for(int x = 0; x < w; x++)
		{	
			if(seg[x][y].bonds[W]) printf("-");
			else printf(" ");
			
			if(seg[x][y].status == OPEN) printf("0");
			else printf(" ");
			
			if(seg[x][y].bonds[E]) printf("-");
			else printf(" ");
		}
	}
	else
	{
		for(int x = 0; x < w; x++)
		{
			if(seg[x][y].bonds[S]) printf(" | ");
			else printf("   ");
		}
	}
}

// Print an entire lattice for visual inspection.
void printLattice(Site **lattice, int s)
{
	for(int y = 0; y < s; y++)
	{
		printf("\n");
		printRow(lattice, y, s, s, HI);
		printf("\n");
		printRow(lattice, y, s, s, MID);
		printf("\n");
		printRow(lattice, y, s, s, LO);
	}
	printf("\n");
}

// Print a segmented lattice for visual inspection.
void printSegments(Site*** segments, int s, int nSegs)
{	
	for(int y = 0; y < s; y++)
	{	
		for(int lvl = 0; lvl < 3; lvl++)
		{
			printf("\n");
			for(int i = 0; i < nSegs; i++)
			{
				if(i == 0) printf(":");
				printRow(segments[i], y, s, WIDTH(s, i, nSegs), lvl);
				printf(":");
			}
		}
	}
	printf("\n");
}

void segmentTest()
{
	int s = 10;
	int nSegs = 4;
	
	printf("\nTest lattice size: %d\n", s);
	printf("Dividing into %d segments.\n", nSegs);
	
	unsigned int seed = time(NULL);
	double*** probs = probsAlloc(s, seed, SITE);
	Site** lattice = segmentAlloc(s, s, 0, probs, 0.3, SITE);
	
	if(ALLOC_ERR(lattice)) printf("Lattice allocation error.\n");
	else
	{
		printf("\nWhole lattice:\n");
		printLattice(lattice, s);
		freeSegment(lattice);
	}
	
	printf("\nSegments:\n");
	
	Site** segments[nSegs];
	bool fail = false;
	
	for(int i = 0; i < nSegs && !fail; i++)
	{
		int w = WIDTH(s, i, nSegs);
		
		segments[i] = segmentAlloc(s, w, MIN_X(s, i, nSegs), probs, 0.5, SITE);
		
		if(ALLOC_ERR(segments[i]))
		{
			fail = true;
			for(int j = 0; j < i; j++) freeSegment(segments[j]);
		}
		else
		{
			int min[2] = {-1, 0};
			int max[2] = {0, 0};
			
			for(int y = 0; y < s; y++)
			{
				for(int x = 0; x < w; x++)
				{
					if(segments[i][x][y].status == OPEN)
					{
						if(min[0] == -1)
						{
							min[0] = x;
							min[1] = y;
						}
						max[0] = x;
						max[1] = y;
					}
				}
			}
			printf("\nTop-left site is (%d,%d).\n", min[0], min[1]);
			printf("Bottom-right site is (%d,%d).\n", max[0], max[1]);
		}
	}
	free(probs);
	
	if(!fail) printSegments(segments, s, nSegs);
	else printf("Error, exiting.\n");
}

void latticeTest()
{
	unsigned int seed = time(NULL);
	int s = 4;
	printf("\nTesting generation with size %d\n", s);
	
	double*** probs = probsAlloc(s, seed, SITE);
	Site** site = segmentAlloc(s, s, 0, probs, 0.5, SITE);
	free(probs);

	probs = probsAlloc(s, seed, BOND);
	Site** bond = segmentAlloc(s, s, 0, probs, 0.5, BOND);
	free(probs);
	
	if(ALLOC_ERR(site)) printf("\nSite seeding lattice allocation fail.\n");
	else
	{
		printf("\nSite seeding lattice test:\n");
		printLattice(site, s);
		
		printf("\nOccupied sites:\n");
		for(int y = 0; y < s; y++) for(int x = 0; x < s; x++) if(site[x][y].status == OPEN) printf("(%d,%d) ", x, y);
		printf("\n");
		
		freeSegment(site);
	}
	
	if(ALLOC_ERR(bond)) printf("\nBond seeding lattice allocation fail.\n");
	else
	{
		printf("\nBond seeding lattice test:\n");
		printLattice(bond, s);
		freeSegment(bond);
	}
}

void sequentialTest()
{
	int s = 8;
	int nSegs = 2;
	unsigned int seed = time(NULL);

	printf("\nTesting sequential search and partition merge with lattice size %d divided into %d segments.\n", s, nSegs);

	double*** probs = probsAlloc(s, seed, SITE);
	Site** segments[nSegs];
	
	for(int i = 0; i < nSegs; i++)
	{
		segments[i] = segmentAlloc(s, WIDTH(s, i, nSegs), MIN_X(s, i, nSegs), probs, 0.7, SITE);
	}
	printSegments(segments, s, nSegs);
	
	Partition results[nSegs];
	
	for(int i = 0; i < nSegs; i++)
	{
		results[i] = sequentialSearch(segments[i], s, WIDTH(s, i, nSegs));
		
		if(PART_ERR(results[i])) printf("Error searching segment %d\n", i);
		else
		{
			printf("\nSegment %d results:\n", i);
			
			for(int j = 0; j < results[i].clusters.size; j++)
			{
				Cluster c = results[i].clusters.data[j];
				printf("Subcluster %d: size %d, hSpan %d, vSpan %d\n", j, c.size, c.xSpan, YSPAN(c));
			}
		}
	}
	
	ClusterList total = mergePartitions(s, nSegs, results);
	
	if(LIST_ERR(total)) printf("Error merging partitions.\n");
	else
	{
		int largest = 0;
		int hSpan = 0;
		int vSpan = 0;
		printf("\n");

		for(int i = 0; i < total.size; i++)
		{
			Cluster c = total.data[i];
			
			printf("Cluster %d: size %d, hSpan %d, vSpan %d.\n", i, c.size, c.xSpan, YSPAN(c));
			
			if(c.size > largest) largest = c.size;
			if(c.xSpan > hSpan) hSpan = c.xSpan;
			if(YSPAN(c) > vSpan) vSpan = YSPAN(c);
		}
		
		printf("\nLargest cluster: %d\n", largest);
		printf("Largest horizontal span: %d\n", hSpan);
		printf("Largest vertical span: %d\n", vSpan);
	}
	
	for(int i = 0; i < nSegs; i++)
	{
		freeSegment(segments[i]);
		freePartition(&results[i]);
	}
		
	free(probs);
	freeList(&total);
}

void parallelTest()
{
	int s = 8;
	int nSegs = 2;
	unsigned int seed = time(NULL);

	printf("\nTesting parallel search on lattice sized %d.\n", s);

	double*** probs = probsAlloc(s, seed, SITE);
	
	if(ALLOC_ERR(probs))
	{
		printf("\nProbs failed to allocate.");
		exit(EXIT_FAILURE);
	}
		
	Site** segs[nSegs];
	
	for(int i = 0; i < nSegs; i++)
	{
		segs[i] = segmentAlloc(s, WIDTH(s, i, nSegs), i*WIDTH(s, i, nSegs), probs, 0.5, SITE);

		if(ALLOC_ERR(segs[i]))
		{
			printf("\nSegment %d failed to allocate.", i);
			exit(EXIT_FAILURE);
		}
	}
	
	printSegments(segs, s, nSegs);

	Partition parts[nSegs];

	for(int i = 0; i < nSegs; i++)
	{
		parts[i] = parallelSearch(segs[i], s, WIDTH(s, i, nSegs));
		
		if(PART_ERR(parts[i]))
		{
			printf("\nSearch of segment %d failed.", i);
			exit(EXIT_FAILURE);
		}

		printf("\n%d subclusters found in segment %d.", parts[i].clusters.size, i);
		
		for(int j = 0; j < parts[i].clusters.size; j++)
		{
			Cluster c = parts[i].clusters.data[j];
			printf("\nSubcluster %d: size %d, hSpan %d, vSpan %d", j, c.size, c.xSpan, YSPAN(c));
		}
	}
	
	ClusterList total = mergePartitions(s, 1, parts);
	
	if(LIST_ERR(total))
	{
		printf("Segment merging failed.\n");
		exit(EXIT_FAILURE);
	}
	
	printf("\nFound %d clusters after merging segments:", total.size);
	for(int i = 0; i < total.size; i++)
	{
		Cluster c = total.data[i];
		printf("\nCluster %d: size %d, hSpan %d, vSpan %d", i, c.size, c.xSpan, YSPAN(c));
	}
	
	for(int i = 0; i < nSegs; i++)
	{
		free(segs[i]);
		freePartition(&parts[i]);
	}
	freeList(&total);
	printf("\n");
	free(probs);
}

void mergeTests()
{
	int s = 8;
	int nSegs = 2;
	unsigned int seed = time(NULL);
	
	printf("\nTesting merge of single segment lattice size %d.", s);
	
	double*** probs = probsAlloc(s, seed, SITE);
	
	if(ALLOC_ERR(probs))
	{
		printf("\nProbs failed to allocate.");
		exit(EXIT_FAILURE);
	}
	
	Site** s1 = segmentAlloc(s, s, 0, probs, 0.5, SITE);
	
	if(ALLOC_ERR(s1))
	{
		printf("\nWhole lattice failed to allocate.");
		exit(EXIT_FAILURE);
	}
	
	printf("\nTesting dfs of one subsegment.\n");
	
	printLattice(s1, s);
	Partition p1 = sequentialSearch(s1, s, s);
	
	if(PART_ERR(p1))
	{
		printf("\nSearch returned error.");
		exit(EXIT_FAILURE);
	}
	
	printf("\nSubclusters found:");
	for(int i = 0; i < p1.clusters.size; i++)
	{
		Cluster c = p1.clusters.data[i];
		printf("\n%d: size %d, xSpan %d, ySpan %d", i, c.size, c.xSpan, YSPAN(c));
	}
	
	printf("\n\nTesting merge of super-partition with itself.\n");
	ClusterList l1 = mergePartitions(s, 1, &p1);
	
	if(LIST_ERR(l1))
	{
		printf("\nMerge returned error.");
		exit(EXIT_FAILURE);
	}
	
	printf("\nClusters found:");
	for(int i = 0; i < l1.size; i++)
	{
		Cluster c = l1.data[i];
		printf("\n%d: size %d, xSpan %d, ySpan %d", i, c.size, c.xSpan, YSPAN(c));
	}
	
		free(s1);
		freePartition(&p1);
		freeList(&l1);
		
	printf("\n\nTesting partition merging with lattice size %d, %d segments.\n", s, nSegs);
	
	Site** s2[2];
	
	s2[0] = segmentAlloc(s, WIDTH(s, 0, nSegs), 0, probs, 0.5, SITE);
	
	if(ALLOC_ERR(s2[0]))
	{
		printf("\nSegment 0 failed to allocate.");
		exit(EXIT_FAILURE);
	}
	
	s2[1] = segmentAlloc(s, WIDTH(s, 1, nSegs), WIDTH(s, 0, nSegs), probs, 0.5, SITE);
	
	if(ALLOC_ERR(s2[1]))
	{
		printf("\nSegment 1 failed to allocate.");
		exit(EXIT_FAILURE);
	}

	printSegments(s2, s, nSegs);
	
	Partition p2[2];
	
	p2[0] = sequentialSearch(s2[0], s, WIDTH(s, 0, nSegs));
	
	if(PART_ERR(p2[0]))
	{
		printf("\nSearching segment 0 return failure.\n");
		exit(EXIT_FAILURE);
	}
	
	p2[1] = sequentialSearch(s2[1], s, WIDTH(s, 1, nSegs));
	
	if(PART_ERR(p2[1]))
	{
		printf("\nSearching segment 0 return failure.\n");
		exit(EXIT_FAILURE);
	}
	
	free(probs);
	free(s2[0]);
	free(s2[1]);
	
	for(int i = 0; i < nSegs; i++)
	{
		printf("\n%d subclusters found in segment %d:", p2[i].clusters.size, i);
		for(int j = 0; j < p2[i].clusters.size; j++)
		{
			Cluster c = p2[i].clusters.data[j];
			printf("\n%d: size %d, xSpan %d, ySpan %d", i, c.size, c.xSpan, YSPAN(c));
		}
		printf("\n");
	}
	
	printf("\nCopying p2 for reuse.");
	Partition p3[nSegs];
	
	for(int i = 0; i < nSegs; i++)
	{
		p3[i] = copyPartition(&p2[i], s);
		
		if(PART_ERR(p3[i]))
		{
			printf("\nCopy failed.");
			exit(EXIT_FAILURE);
		}
		
		p3[i].minX = i*WIDTH(s, i, nSegs);
		p3[i].maxX = (i+1)*WIDTH(s, i, nSegs)-1;
	}
	
	printf("\nTesting merge of segment partitions into whole lattice cluster list.\n");
	ClusterList l2 = mergePartitions(s, nSegs, p2);
	
	if(LIST_ERR(l2))
	{
		printf("\nMerging failed.");
		exit(EXIT_FAILURE);
	}
	
	printf("\n%d clusters found:", l2.size);
	for(int i = 0; i < l2.size; i++)
	{
		Cluster c = l2.data[i];
		printf("\n%d: size %d, xSpan %d, ySpan %d", i, c.size, c.xSpan, YSPAN(c));
	}

	freePartition(&p2[0]);
	freePartition(&p2[1]);
	freeList(&l2);
	
	printf("\n\nTesting merge of subsegments into single partition.\n");

	Partition p4 = mergeSubSegments(s, nSegs, p3);
	
	if(PART_ERR(p4))
	{
		printf("\nMerging subsegments failed.");
		exit(EXIT_FAILURE);
	}

	printf("\n%d subclusters found:", p4.clusters.size);
	for(int i = 0; i < p4.clusters.size; i++)
	{
		Cluster c = p4.clusters.data[i];
		printf("\n%d: size %d, xSpan %d, ySpan %d", i, c.size, c.xSpan, YSPAN(c));
	}

	freePartition(&p3[0]);
	freePartition(&p3[1]);
	freePartition(&p4);
}
