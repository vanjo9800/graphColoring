#ifndef __MASTER_INCLUDED__
#define __MASTER_INCLUDED__

#include<vector>
#include<mpi.h>
#include "input.h"
#include "output.h"
#include "constants.h"

extern const int cyclesMultiplier;
extern double memory[MaxVertexNumber][MaxVertexNumber];
extern double deltaMemory[MaxVertexNumber][MaxVertexNumber];
extern const double evaporationRate;
extern bool neighborsScheme[MaxVertexNumber][MaxVertexNumber];

void initializeMemory();

void initializeDeltaMemory();

void sendMToNodes();

void makeNeighborScheme();

void destructor();

void masterMain(char* outputFile);
#endif 
