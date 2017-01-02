#include "node.h"

double currentMemory[MaxVertexNumber][MaxVertexNumber];

int coloring[MaxVertexNumber+1],newColor[MaxVertexNumber+1];
bool candidatesForColorCheck[MaxVertexNumber],cannotBeColoredCheck[MaxVertexNumber];
int candidatesForColor[MaxVertexNumber],candidatesForColorSize;

int desirability(int vertex,int choosingDesirability){
	if(choosingDesirability==0){
		int desirabilityOfVertex=0;
		for(int i=0;i<neighbors[vertex].size();i++){
			desirabilityOfVertex+=cannotBeColoredCheck[neighbors[vertex][i]];
		}
		return desirabilityOfVertex;
	}
	if(choosingDesirability==1){
		int counter=0;
		for(int i=0;i<neighbors[vertex].size();i++){
			counter+=candidatesForColorCheck[neighbors[vertex][i]];
		}
		return candidatesForColorSize-counter;
	}
	int desirabilityOfVertex=0;
	for(int i=0;i<neighbors[vertex].size();i++){
		desirabilityOfVertex+=cannotBeColoredCheck[neighbors[vertex][i]]||candidatesForColorCheck[neighbors[vertex][i]];
	}
	return desirabilityOfVertex;
}

double trailFactor(int vertex){
	if(newColor[0]==0){
		return 1.0;
	}else{
		double trailFactorForVertex=0.0;
		for(int i=1;i<=newColor[0];i++){
			trailFactorForVertex+=currentMemory[newColor[i]][vertex];
		}
		return trailFactorForVertex/newColor[0];
	}
}

double probability(int vertex,int choosingDesirability){
	return desirability(vertex,choosingDesirability)*trailFactor(vertex);
}

void recursiveLargestFirst(){
	
	for(int i=0;i<vertexNumber;i++){
		MPI_Bcast(currentMemory[i],vertexNumber,MPI_DOUBLE,masterNode,MPI_COMM_WORLD);
	}

	std::mt19937_64 gen(std::random_device{}());
	std::uniform_int_distribution<unsigned> parametersGenerator(1,6);
	int choosingFirstVertex=parametersGenerator(gen)%2;
	int choosingDesirability=parametersGenerator(gen)%3;
	
	int numberOfColors=0;
	candidatesForColorSize=vertexNumber;
	for(int i=0;i<vertexNumber;i++){
		candidatesForColor[i]=i;
		candidatesForColorCheck[i]=1;
	}

	int numberOfColored=0;
	newColor[0]=0;

	while(numberOfColored<vertexNumber){
		
		numberOfColored++;
		numberOfColors++;
		newColor[0]=0;

		int vertex=0,position=0;
		if(choosingFirstVertex==0){
		
			int MaxVertexNumberertexDegree=0;
			for(int i=0;i<candidatesForColorSize;i++){
				
				int counter=0;
				for(int j=0;j<neighbors[candidatesForColor[i]].size();j++){
					counter+=candidatesForColorCheck[neighbors[candidatesForColor[i]][j]];
				}
				if(counter>=MaxVertexNumberertexDegree){
					MaxVertexNumberertexDegree=counter;
					vertex=candidatesForColor[i];
					position=i;
				}
			}
		}
		else{
			std::uniform_int_distribution<unsigned> randomVertex(0,candidatesForColorSize-1);
			position=randomVertex(gen);
			vertex=candidatesForColor[position];
		}
		
		newColor[newColor[0]+1]=vertex;
		newColor[0]++;

		while(true){
			
			int counter=1;
			for(int i=0;i<neighbors[vertex].size();i++){
				counter+=candidatesForColorCheck[neighbors[vertex][i]];
			}

			if(candidatesForColorSize<=counter){
				break;
			}

			numberOfColored++;
			candidatesForColorCheck[vertex]=0;
			std::swap(candidatesForColor[position],candidatesForColor[candidatesForColorSize-1]);
			candidatesForColorSize--;
			
			for(int i=0;i<neighbors[vertex].size();i++){
				cannotBeColoredCheck[neighbors[vertex][i]]|=candidatesForColorCheck[neighbors[vertex][i]];
				candidatesForColorCheck[neighbors[vertex][i]]=false;
			}
			for(int i=0;i<candidatesForColorSize;i++){
				if(!candidatesForColorCheck[candidatesForColor[i]]){
					std::swap(candidatesForColor[i],candidatesForColor[candidatesForColorSize-1]);
					candidatesForColorSize--;
					i--;
				}
			}

			double probabilities[MaxVertexNumber];
			double sumOfProbabilities=0.0;
			for(int i=0;i<candidatesForColorSize;i++){

				probabilities[candidatesForColor[i]]=probability(candidatesForColor[i],choosingDesirability);
				sumOfProbabilities+=probabilities[candidatesForColor[i]];
			}

			std::uniform_real_distribution<double> chooseNext(0.0,sumOfProbabilities);
			double nextVertex=chooseNext(gen),currentSum=0.0;
			for(int i=0;i<candidatesForColorSize;i++){

				currentSum+=probabilities[candidatesForColor[i]];
				if(currentSum>=nextVertex){
					vertex=candidatesForColor[i];
					position=i;
					break;
				}
			}

			newColor[newColor[0]+1]=vertex;
			newColor[0]++;
		}

		for(int i=1;i<=newColor[0];i++){
			coloring[newColor[i]]=numberOfColors;
		}

		for(int i=0;i<neighbors[vertex].size();i++){
			cannotBeColoredCheck[neighbors[vertex][i]]|=candidatesForColorCheck[neighbors[vertex][i]];
		}
		candidatesForColorSize=0;

		for(int i=0;i<vertexNumber;i++){

			candidatesForColorCheck[i]=cannotBeColoredCheck[i];
			if(cannotBeColoredCheck[i]){
				candidatesForColor[candidatesForColorSize++]=i;
			}
			cannotBeColoredCheck[i]=false;
		}

	}
	coloring[vertexNumber]=numberOfColors;
}

void nodeMain(){
	int triggerSignal;
	while(true){
		MPI_Recv(&triggerSignal,1,MPI_INT,masterNode,constructSolutionTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		if(triggerSignal==-1) break;
		recursiveLargestFirst();
		MPI_Send(&coloring,vertexNumber+1,MPI_INT,masterNode,sendSolutionTag,MPI_COMM_WORLD);
	}
}
