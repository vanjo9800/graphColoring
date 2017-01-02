#ifndef __NODE_INCLUDED__
#define __NODE_INCLUDED__

#include<vector>
#include<set>
#include<mpi.h>
#include<random>
#include "input.h"
#include "constants.h"

extern double currentMemory[MaxVertexNumber][MaxVertexNumber];
extern int coloring[MaxVertexNumber+1],newColor[MaxVertexNumber+1];
extern bool candidatesForColorCheck[MaxVertexNumber],cannotBeColoredCheck[MaxVertexNumber];
extern int candidatesForColor[MaxVertexNumber],candidatesForColorSize;
extern int cannotBeColored[MaxVertexNumber];

int desirability(int vertex,int choosingDesirability);

double trailFactor(int vertex,int color);

double probability(int vertex,int color,int choosingDesirability);

void recursiveLargestFirst();

void nodeMain();
#endif 
