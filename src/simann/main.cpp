#include <mpi.h>
#include <random>
#include <math.h>
#include "input.h"

#define masterRank 0

int colorNumber=7;
const double e = 2.718281828;
const int ei = 7;
const double alpha = 0.95;
const double beta = 1.05;

std::mt19937_64 gen{std::random_device{}()};

void generateInitialSolution(int solution[],int &colorNumber){
	std::uniform_int_distribution<unsigned> color(1,colorNumber);
	for(int i=0;i<vertexNumber;i++){
		solution[i]=color(gen);
	}
}

void generateNeighborSolution(int newSolution[], int solutionSource[], int colorNumber){
	
	bool conflictingCheck[vertexNumber];
	int conflicting[vertexNumber],conflictingNumber=0;
	for(int i=0; i<vertexNumber; i++)
	{
		newSolution[i]=solutionSource[i];
		conflictingCheck[i]=false;
	}

	for(int i=0; i<vertexNumber; i++)
	{
		for(int j=0; j<neighbors[i].size(); j++)
		{
			if(solutionSource[i] == solutionSource[neighbors[i][j]])
			{
				if(!conflictingCheck[i]){
					conflicting[conflictingNumber++]=i;
					conflictingCheck[i]=true;
				}
			}
		}

	}

	if(conflictingNumber > 0){
		
		std::uniform_int_distribution<int> vertex(0, conflictingNumber-1);
		int randomVertex = conflicting[vertex(gen)];
		
		std::uniform_int_distribution <int> color(1, colorNumber);
		int randomColor = color(gen);
		newSolution[randomVertex] = randomColor;
	}else{
	
		std::uniform_int_distribution<int> vertex(0, vertexNumber-1);
		int randomVertex = vertex(gen);
		
		std::uniform_int_distribution <int> color(1, colorNumber);
		int randomColor = color(gen);
		newSolution[randomVertex] = randomColor;
	}
}
int calculateCost(int solution[], int colorNumber){

	bool conflict=false;
	int answer=0;

	for(int i=0; i<vertexNumber; i++)
	{
		for(int j=0; j<neighbors[i].size(); j++)
		{
			if(solution[i] == solution[neighbors[i][j]])
			{
				answer++;
				if(!conflict) conflict=true;
			}
		}
	}
	
	answer+=conflict;
	answer+=colorNumber;
	return answer;  

}

int Anneal(double temp, int deltaCost){
	
	std::uniform_real_distribution<double> chance(0.0, 1.0);
	double updateChance = chance(gen);
	double probability = pow(e, -deltaCost/temp);

	if(probability > updateChance){
		return 1;
	}
	if(probability <= updateChance)
	{
		return 0;
	}
}

int main(int argc,char **argv){

	if(argc!=2){
		printf("The application accepts two files as input and output as parameters.\n Example: mpirun -n N bin/simann input.txt answer.txt\n");
		return 0;
	}

	MPI_Init(NULL, NULL);
	int numberOfProcesses;
	MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);
	int processRank;
	MPI_Comm_rank(MPI_COMM_WORLD, &processRank);
	
	readInput(argv[1]);

	int bestCost,targetCost = colorNumber;
	double temperature = 1000;
	double temperatureIterations = 1;
	
	int currentSolution[vertexNumber];
	int currentBestSolutionAtMaster[vertexNumber];
	int costAtMaster = 1<<30;
	int bestSolution[vertexNumber];

	if (processRank == masterRank){
		bestCost = 1<<30;
	}
	else{
		generateInitialSolution(currentSolution,colorNumber);
	}

	int iterationNumber = 10000;//*vertexNumber
	int steps=0; 
	for(int i=0; i<iterationNumber; i++){
	
		if(processRank != masterRank){
			
			int neighborSolution[vertexNumber];
			generateNeighborSolution(neighborSolution, currentSolution, colorNumber);
			int costOld = calculateCost(currentSolution, colorNumber);
			int costNew = calculateCost(neighborSolution, colorNumber);
			if(Anneal(temperature, costNew-costOld) == 1){
				for(int j = 0; j<vertexNumber; j++){
					currentSolution[j] = neighborSolution[j];
				}
			}
			MPI_Send(&currentSolution, vertexNumber, MPI_INT, masterRank, processRank, MPI_COMM_WORLD);
		}
		if(processRank == masterRank){
			
			if(i%10==0) printf("Iteration %d\n",i);
			for(int slave=1; slave<numberOfProcesses; slave++){
				
				int currentSolutonAtMasterReceived[vertexNumber];
				MPI_Recv(&currentSolutonAtMasterReceived, vertexNumber, MPI_INT, slave, slave, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				int costCurrentSolutionAtMaster = calculateCost(currentSolutonAtMasterReceived, colorNumber);
				if(costAtMaster > costCurrentSolutionAtMaster){
					costAtMaster = costCurrentSolutionAtMaster;
					for(int j = 0; j<vertexNumber; j++){
						currentBestSolutionAtMaster[j]=currentSolutonAtMasterReceived[j];
					}
				}
				if(bestCost > costAtMaster){
					bestCost = costAtMaster;
					for(int j=0; j<vertexNumber; j++){
						bestSolution[j] = currentBestSolutionAtMaster[j];
					}
					if(bestCost <= targetCost){
						break;
					}
				}
			}
		}
		if(i%ei == 0){
			if(processRank == masterRank){
				MPI_Bcast(&bestSolution, vertexNumber, MPI_INT, masterRank, MPI_COMM_WORLD);
			}else{
				MPI_Bcast(&currentSolution, vertexNumber, MPI_INT, masterRank, MPI_COMM_WORLD);
			}

		}
		if((i-steps)>int(temperatureIterations)){
			temperature*=alpha;
			steps+=int(temperatureIterations);
		}
		temperatureIterations*=beta;
	}

	if(processRank == masterRank){
		
		for(int i=0;i<vertexNumber;i++){
			for(int j=0;j<neighbors[i].size();j++){
				if(bestSolution[i]==bestSolution[neighbors[i][j]]){
					printf("Conflict between vertex %d and %d\n",i,neighbors[i][j]);
				}
			}
		}

		printf("Best cost %d; Best solution: ",bestCost);
		for(int i=0; i<vertexNumber; i++){
			printf("%d ",bestSolution[i]);
		}
		printf("\n");
	}

	MPI_Finalize();
	return 0;
}
