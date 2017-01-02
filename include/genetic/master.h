#ifndef __MASTER_H_INCLUDED__
#define __MASTER_H_INCLUDED__

#include "input.h"
#include "output.h"
#include "Communication.h"

extern const int populationMultiplier;
extern const int generationMultiplier;
extern const int roundsOfProcessing;
extern const int minimalRandomShuffleBlock;
 
extern Genotype lastSuccessfulColoring;
extern Genotype* population;
extern Genotype* GMPopulation;

bool possibleColoring(int numberOfColors);

void masterMain(char *outputFile);

#endif
