#include <mpi.h>
#include <random>
#include <math.h>
#include "input.h"
#include <algorithm>
#include <map>
#include <deque>

#define masterRank 0

const int tabuSize=7;
const int exchangeSteps=10;

std::map<int,int> aspiration;

int processRank,numberOfProcesses;

std::mt19937_64 gen{std::random_device{}()};

std::deque<std::pair<int,int>> tabuList;

void generateInitialSolution(int solution[],int &colorNumber){
	std::uniform_int_distribution<unsigned> color(1,colorNumber);
	for(int i=0;i<vertexNumber;i++){
		solution[i]=color(gen);
	}
}

int calculateCost(int solution[]){

	int conflicts = 0;

	for(int i=0; i< vertexNumber; i++){
		for(int j=0; j<neighbors[i].size(); j++){
			if(solution[i] == solution[neighbors[i][j]]){
				conflicts++;
			}
		}
	}
	return conflicts;
}

void generateNeighborSolution(int newSolution[], int solutionSource[], int colorNumber, int currentCost, int &newCost){
	
	for(int i=0; i < vertexNumber; i++){
		newSolution[i] = solutionSource[i];
	}

	std::uniform_int_distribution <int> vertex(0, vertexNumber-1);
	std::uniform_int_distribution <int> color(1, colorNumber);
	int randomVertex = vertex(gen);
	int randomColor = color(gen);
	
	bool tabu=false;
	for(int i=0;i<tabuList.size();i++){
		if(tabuList[i].first==randomVertex&&tabuList[i].second==randomColor){
			tabu=true;
		}
	}

	if(tabu){
		newSolution[randomVertex] = randomColor;
		newCost=calculateCost(newSolution);
		if(aspiration.find(currentCost)==aspiration.end()){
			aspiration[currentCost]=currentCost-1;
		}
		if(aspiration[currentCost]>newCost){
			aspiration[currentCost]=newCost;
		}else{
			newCost=-1;
		}
	}else{
		newSolution[randomVertex] = randomColor;
		newCost=calculateCost(newSolution);
	}
	/*bool conflictingCheck[vertexNumber];
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
	}*/
}

int color(int vertex, int colored[],int &colorNumber){
	
	bool colors[vertexNumber];
	for(int i=1;i<=vertexNumber;i++){
		colors[i]=false;
	}
	for(int i=0;i<neighbors[vertex].size();i++){
		colors[colored[neighbors[vertex][i]]]=true;
	}
	for(int k=1;k<=vertexNumber;k++){
		if(!colors[k]){
			if(colorNumber<k) colorNumber=k;
			return k;
		}
	}
}

int estimator(int &colorNumber, int colored[]){
	int ordered[vertexNumber];
	for(int i=0;i<vertexNumber;i++){
		ordered[i]=i;
	}

	std::sort(ordered,ordered+vertexNumber,[](int a,int b){ return neighbors[a].size()>neighbors[b].size();});
	
	for(int i=0;i<vertexNumber;i++){
		int currentVertex=ordered[i];
		if(colored[currentVertex]) continue;
		colored[currentVertex]=color(currentVertex, colored,colorNumber);
		std::sort(neighbors[currentVertex].begin(),neighbors[currentVertex].end(),[](int a,int b){ return neighbors[a].size() > neighbors[b].size();});
		for(int j=0;j<neighbors[currentVertex].size();j++){
			if(colored[neighbors[currentVertex][j]]) continue;
			colored[neighbors[currentVertex][j]]=color(neighbors[currentVertex][j], colored, colorNumber);
		}
	}
}

bool foundSolution(int colorNumber){

	int solved=0;
	int bestSolution[vertexNumber],bestCost=1<<30;
	int currentSolution[vertexNumber],currentSolutionCost=1<<30;
	int iterationNumber = 10000, lookAtNeighbors=vertexNumber/2;

	for(int i=0;i<vertexNumber;i++){
		bestSolution[i]=-100;
		currentSolution[i]=-100;
	}
	if (processRank == masterRank){
		bestCost = 1<<30;
	}

	if(processRank != masterRank){
		generateInitialSolution(currentSolution,colorNumber);
	}
	currentSolutionCost = calculateCost(currentSolution);

	for(int i=0; i<iterationNumber; i++){

		if(processRank != masterRank){
			bestCost=-1;
			for(int j=0;j<lookAtNeighbors;j++){
				int neighborSolution[vertexNumber],newSolutionCost;
				generateNeighborSolution(neighborSolution, currentSolution, colorNumber, currentSolutionCost, newSolutionCost);
				if(newSolutionCost!=-1){
					if(bestCost==-1||bestCost>newSolutionCost){
						for(int p=0;p<vertexNumber;p++){
							bestSolution[p]=neighborSolution[p];
						}
						bestCost=newSolutionCost;
						if(currentSolutionCost>newSolutionCost){
							break;
						}
					}
				}else{
					j--;
				}
			}
			currentSolutionCost=bestCost;
			int br=0;
			for(int p=0;p<vertexNumber;p++){
				if(currentSolution[p]!=bestSolution[p]){
					br++;
					if(tabuList.size()==tabuSize) tabuList.pop_front();
					tabuList.push_back({p,bestSolution[p]});
				}
				currentSolution[p]=bestSolution[p];
			}

			MPI_Send(&colorNumber, 1, MPI_INT, masterRank, processRank, MPI_COMM_WORLD);
			MPI_Send(&currentSolution, vertexNumber, MPI_INT, masterRank, processRank, MPI_COMM_WORLD);
		}
		if(processRank == masterRank){

			for(int slave=1; slave<numberOfProcesses; slave++){
			
				int costAtMaster=1<<30,nodeColorNumber;
				int currentSolutonAtMasterReceived[vertexNumber];
				MPI_Recv(&nodeColorNumber, 1, MPI_INT, slave, slave, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				MPI_Recv(&currentSolutonAtMasterReceived, vertexNumber, MPI_INT, slave, slave, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				int costCurrentSolutionAtMaster = calculateCost(currentSolutonAtMasterReceived);
				if(costAtMaster > costCurrentSolutionAtMaster && nodeColorNumber == colorNumber){
					costAtMaster = costCurrentSolutionAtMaster;
					for(int j = 0; j<vertexNumber; j++){
						currentSolution[j]=currentSolutonAtMasterReceived[j];
					}
				}
				if(bestCost > costAtMaster){
					bestCost = costAtMaster;
					for(int j=0; j<vertexNumber; j++){
						bestSolution[j] = currentSolution[j];
					}
					if(bestCost <= 0){
						break;
					}
				}
			}
		}
		if(i%exchangeSteps==0){
			if(processRank == masterRank){
				MPI_Bcast(currentSolution, vertexNumber, MPI_INT, masterRank, MPI_COMM_WORLD);
			}else{
				MPI_Bcast(currentSolution, vertexNumber, MPI_INT, masterRank, MPI_COMM_WORLD);
				tabuList.clear();
			}
		}
	}

	if(processRank == masterRank){
		if(bestCost == 0){
			printf("Color number: %d\n", colorNumber);
			for(int x=0; x<vertexNumber; x++){
				printf("%d ",bestSolution[x]);
			}
			printf("\n\n");
		}
		else{
			solved = 1;
		}

		MPI_Bcast(&solved, 1, MPI_INT, masterRank, MPI_COMM_WORLD);
	}
	
	if(processRank != masterRank){
		MPI_Bcast(&solved, 1, MPI_INT, masterRank, MPI_COMM_WORLD);
	}

	return solved==0;
}

int main(int argc,char **argv){

	if(argc!=2){
		printf("The application accepts two files as input and output as parameters.\n Example: mpirun -n N bin/tabus input.txt answer.txt\n");
		return 0;
	}

	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);
	MPI_Comm_rank(MPI_COMM_WORLD, &processRank);
	
	readInput(argv[1]);
	
	int colored[vertexNumber],colorNumber; 
	for(int i=0; i<vertexNumber; i++){
		colored[i] = 0;
	}
	
	estimator(colorNumber, colored);
	if(processRank == masterRank){
		printf("Estimator: %d\n",colorNumber);
	}
	
	while(foundSolution(colorNumber-1)) colorNumber--;

	if(processRank == masterRank){
			printf("answer: %d \n", colorNumber);
	}
	MPI_Finalize();
	return 0;
}
