#include<random>
#include<algorithm>
#include<queue>
#include<set>
#include "node.h"

const float genotypeIndividualMutationRate = 0.1;
const float genotypeGlobalMutationRate = 0.2;
const float populationTopFittestReproduction = 0.30;
const float populationRandomReproduction = 0.1;

int fitness(Genotype &individual){
	int conflicts = 0;

	for(int i=0; i<vertexNumber; i++)
	{
		for(int j=0; j<neighbors[i].size();j++)
		{
			if(individual.get(i) == individual.get(neighbors[i][j]))
			{
				conflicts++;
			}
		}
	}
	if(conflicts == 0){
		communication::foundSolution(individual);
	}
	return conflicts;
} 

void generate(Genotype* population,int genotypeNumber,int numberOfColors)
{
	std::mt19937_64 gen(std::random_device{}());
	std::uniform_int_distribution <int> color(1, numberOfColors);

	for(int i=0; i<genotypeNumber; i++)
	{
		for(int j=0; j<vertexNumber; j++)
		{
			(population+i) -> set(j, color(gen));
		}
		(population+i) -> setFitness(fitness(*(population+i)));
	}
}

Genotype crossover(Genotype* firstParent, Genotype* secondParent){

	std::mt19937_64 gen(std::random_device{}());	
	std::uniform_int_distribution <int>  selectionIndex(0, vertexNumber-1);
	std::vector<int> branchPoints;
	std::uniform_int_distribution <int> genNumberOfPoints(1,vertexNumber-1);
	int numberOfBranchPoints=genNumberOfPoints(gen);
	
	for(int i=0;i<numberOfBranchPoints;i++){
		branchPoints.push_back(selectionIndex(gen));
	}
	
	std::sort(branchPoints.begin(),branchPoints.end());
	auto newBranchEnd=std::unique(branchPoints.begin(),branchPoints.end());
	branchPoints.resize(std::distance(branchPoints.begin(),newBranchEnd));
	
	Genotype child;
	int index=0;
	int state=0;

	for(int i=0;i<vertexNumber;i++){

		if(!state){
			child.set(i,(*firstParent).get(i));
		}else{
			child.set(i,(*secondParent).get(i));
		}
		if(i==branchPoints[index]){
			state^=1;
			index++;
		}
	}
	child.setFitness(fitness(child));
	return child;
}

void reproduce(Genotype* population, int sizeOfPopulation, Genotype* children,int &childrenSize)
{
	Genotype child;
	std::mt19937_64 gen(std::random_device{}());	
	std::uniform_int_distribution <int>  selectionIndex(0, vertexNumber-1);
	std::priority_queue<Genotype> topIndividuals;
	std::vector<Genotype> crossoverCandidates;
	int topIndividualsSize=sizeOfPopulation*populationTopFittestReproduction;

	for(int i=0; i<sizeOfPopulation; i++){
		topIndividuals.push(*(population+i));
		if(topIndividuals.size()>topIndividualsSize){
			topIndividuals.pop();
		}
	}

	int randomIndividualsSize=sizeOfPopulation*populationRandomReproduction;

	for(int i=0;i<randomIndividualsSize;i++){
		crossoverCandidates.push_back(*(population+selectionIndex(gen)));
	}

	for(int i=0;i<topIndividualsSize;i++){
		crossoverCandidates.push_back(topIndividuals.top());
		topIndividuals.pop();
	}

	std::shuffle(crossoverCandidates.begin(),crossoverCandidates.end(),gen);

	childrenSize=0;
	for(int i=0;i<topIndividualsSize+randomIndividualsSize-1;i++){
		(*(children + childrenSize) )=crossover(&crossoverCandidates[i],&crossoverCandidates[i+1]);
		childrenSize++;
	}
}

void mutateRandom(Genotype* individual,int numberOfColors)
{
	std::mt19937_64 gen(std::random_device{}());
	std::uniform_int_distribution <int> color(1, numberOfColors);
	std::uniform_int_distribution <int> index(0, vertexNumber-1);	

	for(int i=0; i<vertexNumber*genotypeIndividualMutationRate; i++)
	{
		individual->set(index(gen), color(gen));
	}
	individual -> setFitness(fitness(*individual));
}

void mutateMinimizeConflicts(Genotype* individual,int numberOfColors)
{
	std::mt19937_64 gen(std::random_device{}());	
	std::uniform_int_distribution <int> vertex(0,vertexNumber-1);
	std::uniform_int_distribution <int> color(1,numberOfColors-1);
	
	for(int i=0; i<vertexNumber*genotypeIndividualMutationRate; i++)
	{
		int conflictVertex = vertex(gen);
		int newColor;

		for(int j=0; j<neighbors[conflictVertex].size(); j++)
		{	
			if(individual->get(neighbors[conflictVertex][j]) == individual->get(conflictVertex)) { 
				newColor = individual->get(conflictVertex) + color(gen)-1;
				newColor%=numberOfColors;
				newColor++;
				individual->set(neighbors[conflictVertex][j], newColor);
			}
		}
	}
	individual -> setFitness(fitness(*individual));
}

void randomShuffle(Genotype* population, int populationSize)
{
	std::mt19937_64 gen(std::random_device{}());

	std::shuffle(population,population + populationSize,gen);
}

int GMUpdate(Genotype* population, int populationSize)
{
	int maxFitness=0;
	int individualIndex;
	for(int i=0; i<populationSize; i++)
	{
		int fitnessValue=(*(population+i)).getFitness();
		if(maxFitness < fitnessValue)
		{
			maxFitness=fitnessValue;
			individualIndex = i;
		}
	}
	return individualIndex;
}

void populationMutation(Genotype* population, int populationSize,int numberOfColors){
	
	std::mt19937_64 gen(std::random_device{}());	
	std::uniform_int_distribution <int> randomTypeOfMutation(0,2);
	std::uniform_int_distribution <int> randomIndividual(0, populationSize-1);

	for(int i=0; i< populationSize*genotypeGlobalMutationRate; i++)
	{
		int typeMutation = randomTypeOfMutation(gen);
		if(typeMutation==1){
			mutateRandom(population+randomIndividual(gen),numberOfColors);

		}
		else{
			mutateMinimizeConflicts(population+randomIndividual(gen),numberOfColors);
		}
	}

}

void nodeMain(){

	int populationSize, genotypeNumber,numberOfColors;
	
	Genotype* population=new Genotype[MaxVertexNumber];
	
	while(true){
		int incomingRequest = communication::NodeListener();
	
		if(incomingRequest ==  communication::NodesSendGMTag)
		{
			communication::NodeReadGM(population, populationSize);
			communication::NodeAnswerGM(GMUpdate(population, populationSize));
		}
		if(incomingRequest == communication::NodesSendCreateTag)
		{
			communication::initSolution();
			communication::NodeReadCreate(populationSize,numberOfColors);
			generate(population,populationSize,numberOfColors);
			communication::NodeAnswerCreate(population,populationSize);
		}
		if(incomingRequest == communication::NodesSendMutateTag)
		{
			communication::NodeReadMutate(population, populationSize);
			populationMutation(population, populationSize,numberOfColors);
			communication::NodeAnswerMutate(population, populationSize);
		}
		if(incomingRequest == communication::NodesSendReproduceTag)
		{
			Genotype* children=new Genotype[MaxVertexNumber];
			communication::NodeReadReproduce(population, populationSize);
			int childrenSize;
			reproduce(population, populationSize, children, childrenSize);
			communication::NodeAnswerReproduce(children, childrenSize);
			delete[] children;
		}
		if(incomingRequest == communication::NodesSendRandomTag)
		{
			communication::NodeReadRandomize(population, populationSize);
			randomShuffle(population, populationSize);
			communication::NodeAnswerRandomize(population, populationSize);
		}
		if(incomingRequest == communication::SuspendTag){
			break;
		}
	}

	printf("Node suspended\n");
	delete[] population;
}

int dependencySorted[MaxVertexNumber];
int dependencySortedReverse[MaxVertexNumber];
bool calculatedDependencyLevels=false;

void calculateDependencyLevels(){
	
	int dependencyLevel[MaxVertexNumber];
	int degreeSorting[MaxVertexNumber];

	for(int i=0;i<vertexNumber;i++){
		degreeSorting[i]=i;
		dependencyLevel[i]=0;
		dependencySorted[i]=i;
	}

	std::sort(degreeSorting,degreeSorting+vertexNumber,[](int a,int b){ return neighbors[a].size() > neighbors[b].size(); });

	for(int i=0;i<vertexNumber;i++){
		int currentVertex=degreeSorting[i];
		for(int j=0;j<neighbors[currentVertex].size();j++){
			if(dependencyLevel[currentVertex]<=dependencyLevel[neighbors[currentVertex][j]]){
				dependencyLevel[neighbors[currentVertex][j]]++;
			}
		}
	}

	std::sort(dependencySorted,dependencySorted+vertexNumber,[&dependencyLevel](int a,int b){
		if(dependencyLevel[a]==dependencyLevel[b]){
			return neighbors[a].size()>neighbors[b].size();
		}
		return dependencyLevel[a]<dependencyLevel[b];
	});

	for(int i=0;i<vertexNumber;i++){
		dependencySortedReverse[dependencySorted[i]]=i;
	}

}

void GMCreate(Genotype* population,int blockSize,int numberOfColors){
	if(!calculatedDependencyLevels){
		calculateDependencyLevels();
		calculatedDependencyLevels=true;
	}

	std::mt19937_64 gen(std::random_device{}());

	std::set<int> PossibleColorings[vertexNumber];

	for(int i=0;i<blockSize;i++){
		for(int j=0;j<vertexNumber;j++){
			PossibleColorings[j].clear();
			for(int p=1;p<=numberOfColors;p++){
				PossibleColorings[j].insert(p);
			}
		}

		for(int j=0;j<vertexNumber;j++){

			int currentVertex=dependencySorted[j];
			if(PossibleColorings[currentVertex].size()==0){
				std::uniform_int_distribution<int> color(1,numberOfColors);
				int newColor=color(gen);
				for(int p=0;p<neighbors[currentVertex].size();p++){
					PossibleColorings[dependencySortedReverse[neighbors[currentVertex][p]]].erase(newColor);
				}
				(*(population+i)).set(currentVertex,newColor);
				continue;
			}
			std::uniform_int_distribution<int> color(0,PossibleColorings[currentVertex].size()-1);

			int index=color(gen);
			for(auto possibleColor:PossibleColorings[currentVertex]){
				if(index==0){
					for(int p=0;p<neighbors[currentVertex].size();p++){
						PossibleColorings[dependencySortedReverse[neighbors[currentVertex][p]]].erase(possibleColor);
					}
					(*(population+i)).set(currentVertex,possibleColor);
					break;
				}
				index--;
			}
		}
		(population+i) -> setFitness (fitness(*(population+i)));
	}
}

void GMMain(){

	while(true){
		
		if(communication::GMsuspend()){
			break;
		}
		if(communication::GMcheckTask()){
			int numberToCreate,numberOfColors;
			communication::GMRead(numberToCreate,numberOfColors);
			Genotype* population=new Genotype [numberToCreate];
			GMCreate(population,numberToCreate,numberOfColors);
			communication::GMAnswer(population,numberToCreate);
			delete[] population;
		}
	}
	printf("GM node suspended\n");
}
