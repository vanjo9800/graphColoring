#ifndef __COMMUNICATION_H_INCLUDED__
#define __COMMUNICATION_H_INCLUDED__

#include<mpi.h>
#include<stdint.h>
#include<random>
#include<queue>
#include "Genotype.h"

namespace communication{

	extern const int GMCommunicationTag;
	extern const int GMPopulationReceiveTag;
	extern const char GMStartMessage;
	extern const int GMNode;
	extern const int GMTag;
	extern const int NodesCommunicationTag;
	extern const int NodesSendGMTag;
	extern const int NodesSendCreateTag;
	extern const int NodesSendMutateTag;
	extern const int NodesSendReproduceTag;
	extern const int NodesSendRandomTag;
	extern const int initialNode;
	extern const int MasterNode;
	extern const int solutionFoundTag;
	extern const int SuspendTag;
	extern char TriggerMessage;
	extern bool foundSolutionMarked;

	extern int numberOfSlaves;
	extern int processRank;
	extern char hostname[MPI_MAX_PROCESSOR_NAME];
	extern int hostnameSize;
	extern MPI::Request GMRequest;
	extern MPI_Datatype MPIGenotype;

	void init(int genotypeSize);
	void finish();
	void initSolution();
	bool master();
	bool GM();
	void startGM(int blockSize,int numberOfColors);
	bool checkGM();
	void receiveGM(Genotype* newPopulation);
	void sendNodesGM(Genotype* currentPopulation,Genotype* GMPopulation,int blockSize,int processes);
	void sendCreate(Genotype* population,int blockSize,int processes,int numberOfSlaves);
	void sendMutate(Genotype* population,int blockSize,int processes);
	void sendReproduce(Genotype* population,int blockSize,int processes);
	void sendRandom(Genotype* population,int blockSize,int processes);
	int NodeListener();
	void NodeReadGM(Genotype* population,int blockSize);
	void NodeAnswerGM(int changeIndex);
	void NodeReadCreate(int &sizeToCreate,int &numberOfColors);
	void NodeAnswerCreate(Genotype* population,int blockSize);
	void NodeReadMutate(Genotype* population,int &blockSize);
	void NodeAnswerMutate(Genotype* population,int blockSize);
	void NodeReadReproduce(Genotype* population,int &blockSize);
	void NodeAnswerReproduce(Genotype* population,int blockSize);
	void NodeReadRandomize(Genotype* population,int &blockSize);
	void NodeAnswerRandomize(Genotype* population,int blockSize);
	bool GMsuspend();
	bool GMcheckTask();
	void GMRead(int &blockSize,int &numberOfColors);
	void GMAnswer(Genotype* population,int blockSize);
	void foundSolution(Genotype solution);
	bool checkSolution(int numberOfColors,Genotype &solution);
	void suspendAllThreads();
};

#endif
