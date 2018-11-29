//Each cluster
int colSpan[segWidth];
int rowSpan[latticeSize];
memset(colSpan,0, segWidth*sizeof(int));
memset(rowSpan,0, latticeSize*sizeof(int));

//In DFS
if(colSpan[x] == 0) colSpan[x] == 1;
if(rowSpan[y] == 0) rowSpan[y] == 1;

//When merging
int tempX[sizeof(c1.colSpan)/sizeof(c1.colSpan[0]) + sizeof(c2.colSpan)/sizeof(c2.colSpan[0])];
memcpy(tempX, c1.colSpan, sizeof(c1.colSpan));
memcpy(tempX + sizeof(c1.colSpan)/sizeof(c1.colSpan[0], c2.colSpan, sizeof(c2.colSpan));
for(int i = 0; i < sizeof(rowSpan)/sizeof(rowSpan[0]); i++) if(c1.rowSpan[i] == 0 && c2.rowSpan[i] == 1) c1.rowSpan[i] == 1;

//Percolation check
int colSum, rowSum;
for(int i = 0; i < latticeSize; i++) colSum += colSpan[i];
for(int i = 0; i < latticeSize; i++) rowSum += rowSpan[i];
if(colSum == latticeSize) printf("%s\n", );
if(rowSum == latticeSize) printf("%s\n", );