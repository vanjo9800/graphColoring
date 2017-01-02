#include "master.h"

const int cyclesMultiplier = 10;

//double** memory;
//double** deltaMemory;
double memory[MaxVertexNumber][MaxVertexNumber];
double deltaMemory[MaxVertexNumber][MaxVertexNumber];

const double evaporationRate = 0.5;
//bool** neighborsScheme;
bool neighborsScheme[MaxVertexNumber][MaxVertexNumber];

void initializeMemory(){
	//memory=new double*[vertexNumber];
	for(int i=0; i<vertexNumber; i++){
	//	memory[i]=new double[vertexNumber];
		for(int j=0;j<vertexNumber;j++) memory[i][j] = 1;
	}
}

void initializeDeltaMemory(){
	//deltaMemory=new double*[vertexNumber];
	for(int i=0; i<vertexNumber; i++){
	//	deltaMemory[i]=new double[vertexNumber];
		for(int j=0;j<vertexNumber;j++) deltaMemory[i][j] = 0;
	}
}

void destructor(){
	//delete memory;
	//delete neighborsScheme;
}

void sendMToNodes(){
	for(int i=0;i<vertexNumber;i++){
		MPI_Bcast(memory[i],vertexNumber,MPI_DOUBLE, masterNode, MPI_COMM_WORLD);
	}
}

void makeNeighborScheme(){
	//neighborsScheme=new bool*[vertexNumber];
	for(int i=0; i<vertexNumber; i++){
		//neighborsScheme[i]=new bool[vertexNumber];
		for(int j=0; j<neighbors[i].size(); j++) neighborsScheme[i][j] = 1;
	}
}

void masterMain(char* outputFile){
	int numberOfProcesses;
	MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);

	initializeMemory();
	makeNeighborScheme();

	int currentSolution[MaxVertexNumber+1];
	int answer=vertexNumber;
	int numberOfCycles=cyclesMultiplier*vertexNumber;
	int* finalSolution=new int[MaxVertexNumber];

	for(int i=0; i<numberOfCycles; i++){
		
		initializeDeltaMemory();
		for(int j=1; j<numberOfProcesses; j++){
			int triggerMessage=1;
			MPI_Send(&triggerMessage,1,MPI_INT,j,constructSolutionTag,MPI_COMM_WORLD);
		}
		sendMToNodes();
		for(int j=1;j<numberOfProcesses; j++){
			int currentAnswer;

			MPI_Recv(&currentSolution,vertexNumber+1,MPI_INT,MPI_ANY_SOURCE,sendSolutionTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

			currentAnswer=currentSolution[vertexNumber];
			for(int firstVertex=0;firstVertex<vertexNumber;firstVertex++){
				for(int secondVertex=0;secondVertex<vertexNumber;secondVertex++){
					if(firstVertex!=secondVertex&&currentSolution[firstVertex]==currentSolution[secondVertex]){
						deltaMemory[firstVertex][secondVertex]+=1.0/currentAnswer;
					}
				}
			}
			if(answer > currentAnswer){
				answer = currentAnswer;
				printf("Colored with %d colors: ",answer);
				for(int i=0;i<vertexNumber;i++){
					printf("%d ",currentSolution[i]);
					finalSolution[i]=currentSolution[i];
				}
				printf("\n");
			}
		}
	
		for(int vertex1=0; vertex1<vertexNumber; vertex1++){
			for(int vertex2=0; vertex2<vertexNumber; vertex2++){
				if(neighborsScheme[vertex1][vertex2] == 0){
					memory[vertex1][vertex2] = evaporationRate * memory[vertex1][vertex2] + deltaMemory[vertex1][vertex2];
				}
			}		
		}
		//delete deltaMemory;
	}
	for(int j=1; j<numberOfProcesses; j++){
		int triggerMessage=-1;
		MPI_Send(&triggerMessage,1,MPI_INT,j,constructSolutionTag,MPI_COMM_WORLD);
	}
	destructor();
	printf("The answer is %d\n",answer);
	output(outputFile,answer,finalSolution);
}

