#ifndef __NODE_H_INCLUDED__
#define __NODE_H_INCLUDED__

#include<iostream>
#include<stdint.h>
#include<random>
#include "input.h"
#include"Communication.h"

int fitness(Genotype* individual);

void generate(int genotypeNumber,int numberOfColors);

void reproduce(Genotype* population, int sizeOfPopulation);

void mutateRandom(Genotype* individual, int numberOfColors);

void mutateMinimizeConflicts(Genotype* individual, int numberOfColors);

void randomShuffle(Genotype* population, int poulationSize);

int GMUpdate(Genotype* population, int populationSize);

void populationMutation(Genotype* population, int individualSize, int numberOfColors);

void checkPop();

void nodeMain();

extern int dependencySorted[4001];
extern int dependencySortedReverse[4001];
extern bool calculatedDependencyLevel;

void calculateDependencyLevel();

void GMMain();

#endif
