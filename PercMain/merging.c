#include "project.h"

#define PART(p)				(parts[p])
#define SUB_INT(p)			((p == 0) ? (PART(p).eastY[y]) : (PART(p).westY[y]))
#define SUPER_INT(p)		(records[p][SUB_INT(p)])
#define SUBCLUST(p)			(PART(p).clusters.data[SUB_INT(p)])
#define SUPERCLUST(p)		(list->data[SUPER_INT(p)])

#define NULL_CLUSTER(c)		(c.size == 0 && c.xSpan == 0 && c.minY == 0 && c.maxY == 0)

// Merge one subcluster into another, or a subcluster into a parent cluster.
Cluster merge(Cluster west, Cluster east, int s)
{
	Cluster new = BLANK_CLUSTER;
	new.size = west.size + east.size;
	new.xSpan = west.xSpan + east.xSpan;
	new.minY = (west.minY < east.minY) ? (west.minY) : (east.minY);
	new.maxY = (west.maxY > east.maxY) ? (west.maxY) : (east.maxY);
	
	return new;
}

void freeMergeRecord(int** record, int nParts)
{
	if(!ALLOC_ERR(record))
	{
		for(int i = 0; i < nParts; i++) if(!ALLOC_ERR(record[i])) free(record[i]);
		free(record);
	}
}

int** allocMergeRecord(Partition* parts, int nParts)
{
	int** record = (int**)malloc(nParts * sizeof(int*));
	
	if(ALLOC_ERR(record))
	{
		printf("Merge record alloc failure.\n");
		return NULL;
	}

	for(int i = 0; i < nParts; i++)
	{
		record[i] = (int*)malloc(parts[i].clusters.size * sizeof(int));
		
		if(ALLOC_ERR(record[i]))
		{
			printf("Merge record alloc failure.\n");
			freeMergeRecord(record, nParts);
			return NULL;
		}
		else memset(record[i], -1, parts[i].clusters.size * sizeof(int));
	}
	return record;
}

// Run down the border between two partitions, merging clusters that share bonds
// across the border.
 void mergeBorder(int s, Partition parts[2], int* records[2], ClusterList* list)
{
	// Go down the eastern border.
	for(int y = 0; y < s && !LIST_ERR(*list); y++)
	{
		// There is a subcluster in both partitions that crosses the border at y,
		// both of which don't already belong to a cluster.
		if(SUB_INT(0) != -1 && SUB_INT(1) != -1)
		{
			
			// If neither subcluster has yet been incorporated into a
			// cluster, create a new cluster from them.
			if(SUPER_INT(0) == -1 && SUPER_INT(0) == -1)
			{
				Cluster new = merge(SUBCLUST(0), SUBCLUST(1), s);
				addCluster(new, list);
				SUPER_INT(0) = list->size;
				SUPER_INT(1) = list->size;
			}
			// If one of the subclusters has been incorporated but
			// the other hasn't, merge in the new one.
			else if(SUPER_INT(0) == -1)
			{
				// Merge the unincorporated subcluster into the parent cluster
				// of the other.
				SUPERCLUST(1) = merge(SUBCLUST(0), SUPERCLUST(1), s);
				SUPER_INT(0) = SUPER_INT(1);
			}
			else if(SUPER_INT(1) == -1)
			{
				SUPERCLUST(0) = merge(SUPERCLUST(0), SUBCLUST(1), s);
				SUPER_INT(1) = SUPER_INT(0);
			}
		}	
	}
}

// Convert any unmerged subclusters in a partition into full clusters.
void promoteSubclusters(int s, Partition part, int* record, ClusterList *list)
{
	for(int i = 0; i < part.clusters.size && !LIST_ERR(*list); i++)
	{
		if(record[i] == -1)
		{
			addCluster(part.clusters.data[i], list);
			record[i] = list->size;
		}
	}
}

// Merging the eastmost partition with the westmost is a little trickier,
// since the subclusters in both are already fully promoted.
ClusterList finalBorder(Partition* parts, int** records, ClusterList *list, int s)
{
	ClusterList final = listAlloc(s);
	
	if(LIST_ERR(final))
	{
		printf("\nCreating mega list failed.");
		freeList(&final);
		return BLANK_LIST;
	}
	
	for(int y = 0; y < s && !LIST_ERR(final); y++)
	{
		// Create new clusters from ones that touch.
		if(SUB_INT(0) != -1 && SUB_INT(1) != -1 && SUPER_INT(0) != SUPER_INT(1))
		{
			if(!NULL_CLUSTER(SUPERCLUST(0)) && !NULL_CLUSTER(SUPERCLUST(1)))
			{
				Cluster mega = merge(SUPERCLUST(0), SUPERCLUST(1), s);
				addCluster(mega, &final);
				
				if(LIST_ERR(final))
				{
					printf("\nMerging superclusters failed.");
					freeList(&final);
				}
			
				SUPERCLUST(0) = BLANK_CLUSTER;
				SUPERCLUST(1) = BLANK_CLUSTER;
			}
		}
	}
	
	// Finally, move over the superclusters unaffected by the final border.
	for(int i = 0; i < list->size && !LIST_ERR(final); i++)
	{
		if(!NULL_CLUSTER(list->data[i])) addCluster(list->data[i], &final);
		
		if(LIST_ERR(final))
		{
			printf("\nFinal cluster move failed.");
			freeList(&final);
		}
	}
	
	return final;
}

// The final step of processing, merging together the subclusters contained
// within each partition.
ClusterList mergePartitions(int s, int nParts, Partition* parts)
{
	// The final list of clusters found within the lattice.
	ClusterList list = listAlloc(s);
	
	if(LIST_ERR(list)) printf("Cluster list allocation failure.\n");

	// A record of which cluster each subcluster has been merge into.
	// mergeRecord[i][j] refers to subcluster j in partition i.
	// Its value is the list index of its parent cluster.
	int** record = allocMergeRecord(parts, nParts);
	
	if(ALLOC_ERR(record)) freeList(&list);

	// Iterate through partitions, merging subclusters on the eastern
	// border.
	for(int i = 0; i < nParts-1 && !LIST_ERR(list); i++)
	{	
		if(!LIST_ERR(list))
		{
			mergeBorder(s, (Partition[]){parts[i], parts[i+1]}, (int*[]){record[i], record[i+1]}, &list);
			if(LIST_ERR(list)) printf("\nBorder merge failed.");
		}
		else freeList(&list);
		
		// Any unmerged subclusters are actually free standing clusters,
		// add them to the list.
		if(!LIST_ERR(list))
		{
			promoteSubclusters(s, parts[i], record[i], &list);
			if(LIST_ERR(list)) printf("\nPromoting subclusters failed.");
		}
		else freeList(&list);
	}
	
	ClusterList final = finalBorder((Partition[]){parts[nParts-1], parts[0]}, (int*[]){record[nParts-1], record[0]}, &list, s);
	
	freeMergeRecord(record, nParts);
	freeList(&list);
	
	return final;
}

// Final step carried out by a process before submitting its results, merging
// the subsegments its threads searched into an aggregated structure.
Partition mergeSubSegments(int s, int nSubs, Partition* subs)
{
	if(PART_ERR(subs[0])) return BLANK_PART;
	else if(nSubs < 2) return copyPartition(&subs[0], s);
	else
	{
		Partition merged = partAlloc(s, subs[0].minX, subs[nSubs-1].maxX);
		
		if(PART_ERR(merged)) printf("Partition allocation failure.\n");
		
		int** record = allocMergeRecord(subs, nSubs);
		
		if(ALLOC_ERR(record))
		{
			printf("Local merge record allocation failure.\n");
			freePartition(&merged);
		}
		
		for(int i = 0; i < nSubs-1 && !PART_ERR(merged); i++)
		{
			if(PART_ERR(subs[i]))
			{
				printf("\nSubsegment %d presented error, aborting.", i);
				freePartition(&merged);
			}
			else if(PART_ERR(subs[i+1]))
			{
				printf("\nSubsegment %d presented error, aborting.", i+1);
				freePartition(&merged);
			}
			else
			{
				if(!PART_ERR(merged))
				{
					mergeBorder(s, (Partition[]){subs[i], subs[i+1]}, (int*[]){record[i], record[i+1]}, &merged.clusters);
					if(PART_ERR(merged)) printf("\nBorder merge failed.");
				}
				else freePartition(&merged);
				
				if(!PART_ERR(merged))
				{
					printf("\nPromoting subclusters in subsegment %d.", i);
					
					promoteSubclusters(s, subs[i], record[i], &merged.clusters);
					if(PART_ERR(merged)) printf("\nPromote subclusters failed.");
				}
				else freePartition(&merged);
			}
		}
		
		freeMergeRecord(record, nSubs);
		return merged;
	}
}
