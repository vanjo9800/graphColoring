#include<stdint.h>
#include<random>
#include "master.h"


const int populationMultiplier=10;
const int generationMultiplier=10;
const int roundsOfProcessing=10;
const int minimalRandomShuffleBlock=2;

Genotype lastSuccessfulColoring;
Genotype* population;
Genotype* GMPopulation;

bool possibleColoring(int numberOfColors){

	int generationsNumber=vertexNumber*generationMultiplier;
	int populationSize=vertexNumber*populationMultiplier;
	int blockSize=populationSize/roundsOfProcessing;
	
	population=new Genotype[populationSize];
	printf("PopulationSize %d\n",populationSize);
	GMPopulation=new Genotype[roundsOfProcessing];
	
	std::mt19937_64 gen(std::random_device{}());
	std::uniform_int_distribution<int> randomShuffleBlock(minimalRandomShuffleBlock,blockSize);
	
	communication::startGM(roundsOfProcessing,numberOfColors);
	communication::sendCreate(population,blockSize,roundsOfProcessing,numberOfColors);
	for(int i=0;i<generationsNumber;i++){
		
		if(i%10==0) printf("Generation: %d\n",i);
		printf("Sending reproduce\n");
		communication::sendReproduce(population,blockSize,roundsOfProcessing);
		printf("Sending mutate\n");
		communication::sendMutate(population,blockSize,roundsOfProcessing);
		if(communication::checkGM()){

			printf("Entering genetic modifier\n");
			communication::receiveGM(GMPopulation);
			communication::startGM(roundsOfProcessing,numberOfColors);
			communication::sendNodesGM(population,GMPopulation,blockSize,roundsOfProcessing);
			printf("Exiting genetic modifier\n");
		}
		//communication::sendRandom(population,randomShuffleBlock(gen),roundsOfProcessing);
		printf("Checking for solution\n");
		if(communication::checkSolution(numberOfColors,lastSuccessfulColoring)){
			for(int i=0;i<vertexNumber;i++){
				printf("%d ",lastSuccessfulColoring.get(i));
			}
			printf("\n");
			//output(lastSuccessfulColoring);
			delete[] population;
			delete[] GMPopulation;
			return true;
		}
	}
	delete[] population;
	delete[] GMPopulation;
	return false;
}

void masterMain(char *outputFile){
	int lowestColorNumber=1,biggestColorNumber=0,middleColorNumber;
	for(int i=0;i<vertexNumber;i++){
		if(biggestColorNumber<neighbors[i].size()+1){
			biggestColorNumber=neighbors[i].size()+1;
		}
	}
	while(lowestColorNumber<biggestColorNumber){
		middleColorNumber=(lowestColorNumber+biggestColorNumber)/2;
		printf("Checking %d\n",middleColorNumber);
		if(possibleColoring(middleColorNumber)){
			biggestColorNumber=middleColorNumber;
		}else{
			lowestColorNumber=middleColorNumber+1;
		}
	}

	int* answerColoring=new int[vertexNumber];
	for(int i=0;i<vertexNumber;i++){
		answerColoring[i]=lastSuccessfulColoring.get(i);
	}
	output(outputFile, biggestColorNumber, answerColoring);
	communication::suspendAllThreads();
}
