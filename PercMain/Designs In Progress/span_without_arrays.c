//Memory permitted, I still think using an array is less messy in keeping track of the spans.

//DFS
//Keep track of the north/south movement for each search

//Always keep track of the northern most edge of the cluster with minY
//If you move north, check that minY is smaller, or it wraps around, and write new edge y-coord
if(movement == MOVE_NORTH && (minY > N || N == s-1)) minY = yAbove;

//Always keep track of the southern most edge of the cluster with maxY
//If you move south, check that maxY is larger, or it wraps around, and write new edge y-coord
if(movement == MOVE_SOUTH && (maxY < S || S == 0)) maxY = yBelow;

//It is possible that minY is larger than maxY, due to wrap arounds

//MERGING
//Check for wrap arounds
bool c1wrap = false;
bool c2wrap = false;
if(c1.minY > c1.maxY) c1wrap = true;
if(c2.minY > c2.maxY) c2wrap = true;

//If neither wraps around
if(!c1wrap && !c2wrap)
{
	if(c1.minY < c2.minY) new.minY = c1.minY;
	else new.minY = c2.minY;
	if(c1.maxY > c2.maxY) new.maxY = c1.maxY;
	else new.maxY = c2.maxY;
}

//If one or more wraps around
else
{
	//Temp values
	int c1minY;
	int c1maxY;
	int c2minY;
	int c2maxY;
	//Unwrap the clusters to see which reaches the furthest linearly
	//Unwrap c1
	if(c1wrap)
	{
		c1minY = c1.minY;
		c1maxY = c1.maxY + s;
	}
	//Unwrap c2
	if(c2wrap)
	{
		c2minY = c2.minY;
		c2maxY = c2.maxY + s;
	}

	//Find the lower minY of the two, then store the wrapped value
	if(c1minY < c2minY) new.minY = c1.minY;
	else new.minY = c2.minY;

	//Find the higher maxY of the two, then store the wrapped value
	if(c1maxY > c2maxY) new.maxY = c1.maxY;
	else new.maxY = c2.maxY;
}