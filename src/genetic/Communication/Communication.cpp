#include "Communication.h"

namespace communication{
	const int GMCommunicationTag=0;
	const int GMPopulationReceiveTag=6;
	const char GMStartMessage='1';
	const int GMNode=1;
	const int GMTag=0;
	const int NodesCommunicationTag=0;
	const int NodesSendGMTag=1;
	const int NodesSendCreateTag=2;
	const int NodesSendMutateTag=3;
	const int NodesSendReproduceTag=4;
	const int NodesSendRandomTag=5;
	const int initialNode=2;
	const int MasterNode=0;
	const int solutionFoundTag=9;
	const int SuspendTag=7;
	char TriggerMessage='1';
	bool foundSolutionMarked;

	int numberOfSlaves;
	int processRank;
	char hostname[MPI_MAX_PROCESSOR_NAME];
	int hostnameSize;
	MPI::Request GMRequest;
	MPI_Datatype MPIGenotype;

	void init(int genotypeSize){
		MPI_Init(NULL, NULL);

		MPI_Comm_size(MPI_COMM_WORLD,&numberOfSlaves);
		numberOfSlaves-=2;
		
		MPI_Comm_rank(MPI_COMM_WORLD, &processRank);

		MPI_Get_processor_name(hostname, &hostnameSize);

		//Genotype a;
		int blocklen[2] = {MaxVertexNumber,1};
		MPI_Datatype type[2] = {MPI_INT,MPI_INT};
		MPI_Aint displacement[2];
		displacement[0]=0;
		displacement[1]=MaxVertexNumber*4;
		//reinterpret_cast<const unsigned char*>(&a.fitness) - reinterpret_cast<const unsigned char*>(&a);

		MPI_Type_create_struct(2,blocklen,displacement,type,&MPIGenotype);
	
		MPI_Type_commit(&MPIGenotype);

		printf("Process with with rank %d, started on host %s\n",processRank,hostname);
	}
	
	void finish(){
		MPI_Finalize();
	}

	void initSolution(){
		foundSolutionMarked=false;
	}

	bool master(){
		return processRank==MasterNode; 
	}

	bool GM(){
		return processRank==GMNode; 
	}

	void startGM(int numberToCreate,int numberOfColors){
		MPI_Send(&numberToCreate,1,MPI_INT,GMNode,GMTag,MPI_COMM_WORLD);
		MPI_Send(&numberOfColors,1,MPI_INT,GMNode,GMTag,MPI_COMM_WORLD);
	}

	bool checkGM(){
		int checkResult;
		MPI_Iprobe(GMNode,GMPopulationReceiveTag,MPI_COMM_WORLD, &checkResult,MPI_STATUS_IGNORE);
		return checkResult==1;
	}

	void receiveGM(Genotype* newPopulation){
		MPI_Status receiveStatus;
		MPI_Probe(GMNode,GMPopulationReceiveTag,MPI_COMM_WORLD,&receiveStatus);
		int GMPopulationCount;
		MPI_Get_count(&receiveStatus,MPIGenotype, &GMPopulationCount);
		MPI_Recv(newPopulation,GMPopulationCount,MPIGenotype,GMNode,GMPopulationReceiveTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	}

	void sendNodesGM(Genotype* currentPopulation,Genotype* GMPopulation,int blockSize,int processes){

		int NodePlace[initialNode+numberOfSlaves];
		for(int i=0;i<numberOfSlaves;i++){
			MPI_Send(currentPopulation+blockSize*i,blockSize,MPIGenotype,i+initialNode,NodesSendGMTag,MPI_COMM_WORLD);
			NodePlace[i+initialNode]=i;
		}
	
		MPI_Status NodeStatus;
		int indexToReplace;
		for(int i=numberOfSlaves;i<processes;i++){
			MPI_Probe(MPI_ANY_SOURCE,NodesSendGMTag,MPI_COMM_WORLD,&NodeStatus);
			MPI_Recv(&indexToReplace,1,MPI_INT,NodeStatus.MPI_SOURCE,NodesSendGMTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			*(currentPopulation+blockSize*NodePlace[NodeStatus.MPI_SOURCE]+indexToReplace)=*(GMPopulation+NodePlace[NodeStatus.MPI_SOURCE]);
			
			NodePlace[NodeStatus.MPI_SOURCE]=i;
			MPI_Send(currentPopulation+blockSize*i,blockSize,MPIGenotype,NodeStatus.MPI_SOURCE,NodesSendGMTag,MPI_COMM_WORLD);
		}

		for(int i=0;i<numberOfSlaves;i++){
			MPI_Probe(MPI_ANY_SOURCE,NodesSendGMTag,MPI_COMM_WORLD,&NodeStatus);
			MPI_Recv(&indexToReplace,1,MPI_INT,NodeStatus.MPI_SOURCE,NodesSendGMTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			*(currentPopulation+blockSize*NodePlace[NodeStatus.MPI_SOURCE]+indexToReplace)=*(GMPopulation+NodePlace[NodeStatus.MPI_SOURCE]);
		}
	}

	void sendCreate(Genotype* population,int blockSize,int processes,int numberOfColors){

		int NodePlace[numberOfSlaves+initialNode];
		for(int i=0;i<numberOfSlaves;i++){
			MPI_Send( &blockSize,1,MPI_INT,i+initialNode,NodesSendCreateTag,MPI_COMM_WORLD);
			MPI_Send( &numberOfColors,1,MPI_INT,i+initialNode,NodesSendCreateTag,MPI_COMM_WORLD);
			NodePlace[i+initialNode]=i;
		}
		
		MPI_Status NodeStatus;
		for(int i=numberOfSlaves;i<processes;i++){
			
			MPI_Probe(MPI_ANY_SOURCE,NodesSendCreateTag,MPI_COMM_WORLD,&NodeStatus);
			MPI_Recv(population+blockSize*NodePlace[NodeStatus.MPI_SOURCE],blockSize,MPIGenotype,NodeStatus.MPI_SOURCE,NodesSendCreateTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

			NodePlace[NodeStatus.MPI_SOURCE]=i;
			MPI_Send( &blockSize,1,MPI_INT,NodeStatus.MPI_SOURCE,NodesSendCreateTag,MPI_COMM_WORLD);
			MPI_Send( &numberOfColors,1,MPI_INT,NodeStatus.MPI_SOURCE,NodesSendCreateTag,MPI_COMM_WORLD);
		}
		
		for(int i=0;i<numberOfSlaves;i++){
			MPI_Probe(MPI_ANY_SOURCE,NodesSendCreateTag,MPI_COMM_WORLD,&NodeStatus);
			MPI_Recv(population+blockSize*NodePlace[NodeStatus.MPI_SOURCE],blockSize,MPIGenotype,NodeStatus.MPI_SOURCE,NodesSendCreateTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		}
	}

	void sendMutate(Genotype* population,int blockSize,int processes){

		int NodePlace[numberOfSlaves];
		
		for(int i=0;i<numberOfSlaves;i++){
			MPI_Send(population+blockSize*i,blockSize,MPIGenotype,i+initialNode,NodesSendMutateTag,MPI_COMM_WORLD);
			NodePlace[i+initialNode]=i;
		}
		
		MPI_Status NodeStatus;
		for(int i=numberOfSlaves;i<processes;i++){
			MPI_Probe(MPI_ANY_SOURCE,NodesSendMutateTag,MPI_COMM_WORLD,&NodeStatus);
			MPI_Recv(population+blockSize*NodePlace[NodeStatus.MPI_SOURCE],blockSize,MPIGenotype,NodeStatus.MPI_SOURCE,NodesSendMutateTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

			NodePlace[NodeStatus.MPI_SOURCE]=i;
			MPI_Send(population+blockSize*i,blockSize,MPIGenotype,NodeStatus.MPI_SOURCE,NodesSendMutateTag,MPI_COMM_WORLD);
		}

		for(int i=0;i<numberOfSlaves;i++){
			MPI_Probe(MPI_ANY_SOURCE,NodesSendMutateTag,MPI_COMM_WORLD,&NodeStatus);
			MPI_Recv(population+blockSize*NodePlace[NodeStatus.MPI_SOURCE],blockSize,MPIGenotype,NodeStatus.MPI_SOURCE,NodesSendMutateTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		}
	}

	void sendReproduce(Genotype* population,int blockSize,int processes){

		//kids replace random
		std::mt19937_64 gen(std::random_device{}());
		std::uniform_int_distribution<int> randomIndividual(0,blockSize-1);
		int NodePlace[numberOfSlaves];
		for(int i=0;i<numberOfSlaves;i++){
			MPI_Send(population+blockSize*i,blockSize,MPIGenotype,i+initialNode,NodesSendReproduceTag,MPI_COMM_WORLD);
			NodePlace[i+initialNode]=i;
		}
		
		MPI_Status NodeStatus;
		Genotype* children=new Genotype[MaxVertexNumber];
		int childrenSize;
		for(int i=numberOfSlaves;i<processes;i++){
			MPI_Probe(MPI_ANY_SOURCE,NodesSendReproduceTag,MPI_COMM_WORLD,&NodeStatus);
			MPI_Get_count(&NodeStatus,MPIGenotype,&childrenSize);
			MPI_Recv(children,childrenSize,MPIGenotype,NodeStatus.MPI_SOURCE,NodesSendReproduceTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			for(int j=0;j<childrenSize;j++){
				(*(population+blockSize*NodePlace[NodeStatus.MPI_SOURCE]+randomIndividual(gen)))=(*(children+j));
			}

			NodePlace[NodeStatus.MPI_SOURCE]=i;
			MPI_Send(population+blockSize*i,blockSize,MPIGenotype,NodeStatus.MPI_SOURCE,NodesSendReproduceTag,MPI_COMM_WORLD);
		}

		for(int i=0;i<numberOfSlaves;i++){
			MPI_Probe(MPI_ANY_SOURCE,NodesSendReproduceTag,MPI_COMM_WORLD,&NodeStatus);
			MPI_Get_count(&NodeStatus,MPIGenotype,&childrenSize);
			MPI_Recv(children,childrenSize,MPIGenotype,NodeStatus.MPI_SOURCE,NodesSendReproduceTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			for(int j=0;j<childrenSize;j++){
				(*(population+blockSize*NodePlace[NodeStatus.MPI_SOURCE]+randomIndividual(gen)))=(*(children+j));
			}
		}
		delete[] children;
	}

	void sendRandom(Genotype* population,int blockSize,int processes){

		int NodePlace[numberOfSlaves];
		
		for(int i=0;i<numberOfSlaves;i++){
			MPI_Send(population+blockSize*i,blockSize,MPIGenotype,i+initialNode,NodesSendRandomTag,MPI_COMM_WORLD);
			NodePlace[i+initialNode]=i;
		}
		
		MPI_Status NodeStatus;
		for(int i=numberOfSlaves;i<processes;i++){
			MPI_Probe(MPI_ANY_SOURCE,NodesSendRandomTag,MPI_COMM_WORLD,&NodeStatus);
			MPI_Recv(population+blockSize*NodePlace[NodeStatus.MPI_SOURCE],blockSize,MPIGenotype,NodeStatus.MPI_SOURCE,NodesSendRandomTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			NodePlace[NodeStatus.MPI_SOURCE]=i;
			MPI_Send(population+blockSize*i,blockSize,MPIGenotype,NodeStatus.MPI_SOURCE,NodesSendRandomTag,MPI_COMM_WORLD);
		}

		for(int i=0;i<numberOfSlaves;i++){
			MPI_Probe(MPI_ANY_SOURCE,NodesSendRandomTag,MPI_COMM_WORLD,&NodeStatus);
			MPI_Recv(population+blockSize*NodePlace[NodeStatus.MPI_SOURCE],blockSize,MPIGenotype,NodeStatus.MPI_SOURCE,NodesSendRandomTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		}
	}

	int NodeListener(){
		MPI_Status incomingTask;
		MPI_Probe(MasterNode,MPI_ANY_TAG,MPI_COMM_WORLD,&incomingTask);
		return incomingTask.MPI_TAG;
	}

	void NodeReadGM(Genotype* population,int blockSize){
		MPI_Status receiveStatus;
		MPI_Probe(MasterNode,NodesSendGMTag,MPI_COMM_WORLD,&receiveStatus);
		MPI_Get_count(&receiveStatus,MPIGenotype, &blockSize);
		MPI_Recv(population,blockSize,MPIGenotype,MasterNode,NodesSendGMTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	}

	void NodeAnswerGM(int changeIndex){
		MPI_Send(&changeIndex,1,MPI_INT,MasterNode,NodesSendGMTag,MPI_COMM_WORLD);
	}

	void NodeReadCreate(int &sizeToCreate,int &numberOfColors){
		MPI_Recv(&sizeToCreate,1,MPI_INT,MasterNode,NodesSendCreateTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		MPI_Recv(&numberOfColors,1,MPI_INT,MasterNode,NodesSendCreateTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	}

	void NodeAnswerCreate(Genotype* population,int blockSize){
		MPI_Send(population,blockSize,MPIGenotype,MasterNode,NodesSendCreateTag,MPI_COMM_WORLD);
	}

	void NodeReadMutate(Genotype* population,int &blockSize){
		MPI_Status receiveStatus;
		MPI_Probe(MasterNode,NodesSendMutateTag,MPI_COMM_WORLD,&receiveStatus);
		MPI_Get_count(&receiveStatus,MPIGenotype, &blockSize);
		MPI_Recv(population,blockSize,MPIGenotype,MasterNode,NodesSendMutateTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	}

	void NodeAnswerMutate(Genotype* population,int blockSize){
		MPI_Send(population,blockSize,MPIGenotype,MasterNode,NodesSendMutateTag,MPI_COMM_WORLD);
	}

	void NodeReadReproduce(Genotype* population,int &blockSize){
		MPI_Status receiveStatus;
		MPI_Probe(MasterNode,NodesSendReproduceTag,MPI_COMM_WORLD,&receiveStatus);
		MPI_Get_count(&receiveStatus,MPIGenotype, &blockSize);
		MPI_Recv(population,blockSize,MPIGenotype,MasterNode,NodesSendReproduceTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	}

	void NodeAnswerReproduce(Genotype* population,int blockSize){
		MPI_Send(population,blockSize,MPIGenotype,MasterNode,NodesSendReproduceTag,MPI_COMM_WORLD);
	}

	void NodeReadRandomize(Genotype* population,int &blockSize){
		MPI_Status receiveStatus;
		MPI_Probe(MasterNode,NodesSendRandomTag,MPI_COMM_WORLD,&receiveStatus);
		MPI_Get_count(&receiveStatus,MPIGenotype, &blockSize);
		MPI_Recv(population,blockSize,MPIGenotype,MasterNode,NodesSendRandomTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

	}

	void NodeAnswerRandomize(Genotype* population,int blockSize){
		MPI_Send(population,blockSize,MPIGenotype,MasterNode,NodesSendRandomTag,MPI_COMM_WORLD);
	}

	bool GMsuspend(){
		int incomingPacket;
		MPI_Iprobe(MasterNode,SuspendTag,MPI_COMM_WORLD, &incomingPacket,MPI_STATUS_IGNORE);
		return incomingPacket == 1;
	}

	bool GMcheckTask(){
		int incomingPacket;
		MPI_Iprobe(MasterNode,GMTag,MPI_COMM_WORLD, &incomingPacket,MPI_STATUS_IGNORE);
		return incomingPacket == 1;
	}

	void GMRead(int &numberToCreate,int &numberOfColors){
		MPI_Recv(&numberToCreate,1,MPI_INT,MasterNode,GMTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		MPI_Recv(&numberOfColors,1,MPI_INT,MasterNode,GMTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	}

	void GMAnswer(Genotype* population,int numberToCreate){
		MPI_Request sample;
		MPI_Isend(population,numberToCreate,MPIGenotype,MasterNode,GMPopulationReceiveTag,MPI_COMM_WORLD,&sample);
	}

	void foundSolution(Genotype solution){
		if(!foundSolutionMarked){
			MPI_Request sample;
			MPI_Isend(&solution,1,MPIGenotype,MasterNode,solutionFoundTag,MPI_COMM_WORLD,&sample);
			foundSolutionMarked=true;
		}	
	}

	bool checkSolution(int numberOfColors,Genotype &solution){
		int incomingPacket;
		MPI_Iprobe(MPI_ANY_SOURCE,solutionFoundTag,MPI_COMM_WORLD, &incomingPacket,MPI_STATUS_IGNORE);
		if(incomingPacket!=1) return false;
		MPI_Recv(&solution,1,MPIGenotype,MPI_ANY_SOURCE,solutionFoundTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		for(int i=0;i<vertexNumber;i++){
			if(solution.get(i) > numberOfColors){
				return false;
			}
		}
		return true;
	}

	void suspendAllThreads(){
		MPI_Send(&TriggerMessage,1,MPI_CHAR,GMNode,SuspendTag,MPI_COMM_WORLD);
		for(int i=0;i<numberOfSlaves;i++){
			MPI_Send(&TriggerMessage,1,MPI_CHAR,i+initialNode,SuspendTag,MPI_COMM_WORLD);
		}
	}
}
