#include<stdint.h>
#include<random>
#include<algorithm>
#include "master.h"

const int populationMultiplier=10;
const int generationMultiplier=10;
const int roundsOfProcessing=10;
const int minimalRandomShuffleBlock=2;

Genotype lastSuccessfulColoring;
Genotype* population;
Genotype* GMPopulation;

bool possibleColoring(int numberOfColors){

	clock_t t;
	t=clock();
	int generationsNumber=vertexNumber*generationMultiplier;
	int populationSize=vertexNumber*populationMultiplier;
	int blockSize=populationSize/roundsOfProcessing;
	
	population=new Genotype[populationSize];
	GMPopulation=new Genotype[roundsOfProcessing];
	
	//std::mt19937_64 gen(std::random_device{}());
	//std::uniform_int_distribution<int> randomShuffleBlock(minimalRandomShuffleBlock,blockSize);
	
	t=clock()-t;
//	printf("Created populations %.6f\n",double(t)/CLOCKS_PER_SEC);

	t=clock();
	communication::startGM(roundsOfProcessing,numberOfColors);
	communication::sendCreate(population,blockSize,roundsOfProcessing,numberOfColors);

	t=clock()-t;
//	printf("Beginning generations %.6f\n",double(t)/CLOCKS_PER_SEC);
	int a;
	//scanf("%d",&a);
	for(int i=0;i<generationsNumber;i++){
		
		if(i%10==0) printf("Generation: %d\n",i);

		t=clock();
		communication::sendReproduce(population,blockSize,roundsOfProcessing);
		t=clock()-t;
//		printf("Reproducing %.6f\n",double(t)/CLOCKS_PER_SEC);

		t=clock();
		communication::sendMutate(population,blockSize,roundsOfProcessing);
		t=clock()-t;
//		printf("Mutating %.6f\n",double(t)/CLOCKS_PER_SEC);
	
		t=clock();
		if(communication::checkGM()){
			communication::receiveGM(GMPopulation);
			communication::startGM(roundsOfProcessing,numberOfColors);
			communication::sendNodesGM(population,GMPopulation,blockSize,roundsOfProcessing);
		}
		t=clock()-t;
//		printf("Genetic modifier %.6f\n",double(t)/CLOCKS_PER_SEC);
		//communication::sendRandom(population,randomShuffleBlock(gen),roundsOfProcessing);
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

int colored[MaxVertexNumber],usedColors=0;
int color(int vertex){

	bool colors[MaxVertexNumber];
	for(int i=1;i<=vertexNumber;i++){
		colors[i]=false;
	}
	for(int i=0;i<neighbors[vertex].size();i++){
		colors[colored[neighbors[vertex][i]]]=true;
	}
	for(int i=1;i<=vertexNumber;i++){
		if(!colors[i]){
			if(usedColors<i) usedColors=i;
			return i;
		}
	}
}

void estimator(int &biggestColorNumber){
	int ordered[MaxVertexNumber];
	for(int i=0;i<vertexNumber;i++){
		ordered[i]=i;
	}
	std::sort(ordered,ordered+vertexNumber,[](int a,int b){ return neighbors[a].size()>neighbors[b].size();});

	bool colors[MaxVertexNumber];
	for(int i=0;i<vertexNumber;i++){
		int currentVertex=ordered[i];
		if(colored[currentVertex]) continue;
		colored[currentVertex]=color(currentVertex);
		std::sort(neighbors[currentVertex].begin(),neighbors[currentVertex].end(),[](int a,int b){ return neighbors[a].size() > neighbors[b].size();});
		for(int j=0;j<neighbors[currentVertex].size();j++){
			if(colored[neighbors[currentVertex][j]]) continue;
			colored[neighbors[currentVertex][j]]=color(neighbors[currentVertex][j]);
		}
	}

	for(int i=0;i<vertexNumber;i++){
		lastSuccessfulColoring.set(i,colored[i]);
	}

	biggestColorNumber=usedColors;
	printf("ESTIMATOR: %d\n",usedColors);
}

void masterMain(char *outputFile){
	//int lowestColorNumber=1,biggestColorNumber=0,middleColorNumber;
	int biggestColorNumber;
	/*for(int i=0;i<vertexNumber;i++){
		if(biggestColorNumber<neighbors[i].size()){
			biggestColorNumber=neighbors[i].size();
		}
	}*/

	estimator(biggestColorNumber);
	while(biggestColorNumber>0&&possibleColoring(biggestColorNumber-1)) biggestColorNumber--;
	//scanf("%d",&usedColors);

	/*biggestColorNumber++;
	while(lowestColorNumber<biggestColorNumber){
		middleColorNumber=(lowestColorNumber+biggestColorNumber)/2;
		printf("Checking %d\n",middleColorNumber);
		if(possibleColoring(middleColorNumber)){
			biggestColorNumber=middleColorNumber;
		}else{
			lowestColorNumber=middleColorNumber+1;
		}
	}*/

	int* answerColoring=new int[vertexNumber];
	for(int i=0;i<vertexNumber;i++){
		answerColoring[i]=lastSuccessfulColoring.get(i);
	}
	output(outputFile, biggestColorNumber, answerColoring);
	communication::suspendAllThreads();
}
