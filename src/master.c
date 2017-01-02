#include <e-hal.h>
#include <stdio.h>
#include <time.h>

#define  MaxVertexNumber 201
typedef unsigned short mint;

const mint cyclesMultiplier = 3;

float memory[MaxVertexNumber][MaxVertexNumber];
float deltaMemory[MaxVertexNumber][MaxVertexNumber];

const float evaporationRate = 0.5;

mint finalSolution[MaxVertexNumber];
mint solution[MaxVertexNumber+1];

mint vertexNumber;
mint** neighbors;
char neighMatrix[MaxVertexNumber][MaxVertexNumber];
mint neighCount[MaxVertexNumber];

unsigned offsets[MaxVertexNumber];
e_mem_t neighborsExternal, vertexNumberExternal, offsetsExternal, memoryExternal, messages, coresProcesses;
int rc;
char *messageText;

void input(char* inputFile){

	const char commentSymbol='c';
	const char graphNodesSymbol='p';
	const char edgeSymbol='e';
	unsigned edgesNumber;

	char beginningSymbol;
	FILE *f = fopen(inputFile,"r");

	fscanf(f,"%c",&beginningSymbol);
	while(beginningSymbol==commentSymbol){
		while(beginningSymbol!='\n'){
			fscanf(f,"%c",&beginningSymbol);
		}
		fscanf(f,"%c",&beginningSymbol);
	}

	if(beginningSymbol==graphNodesSymbol){
		fscanf(f,"%c",&beginningSymbol);
		do{
			fscanf(f,"%c",&beginningSymbol);
		}while(beginningSymbol!=' ');

		fscanf(f,"%d",&vertexNumber);
		fscanf(f,"%d",&edgesNumber);
	}
	rc = e_shm_alloc(&vertexNumberExternal,"vertexNumber",sizeof(mint));
	if(rc != E_OK){
	//	e_shm_release("vertexNumber");
		//rc = e_shm_alloc(&vertexNumberExternal,"vertexNumber",sizeof(int));
		rc=e_shm_attach(&vertexNumberExternal,"vertexNumber");
		if(rc != E_OK){
			printf("Failed vertexNumber memory access\n");
		}
	}
	e_write(&vertexNumberExternal,0,0,0,&vertexNumber,sizeof(mint));

	fscanf(f,"%c",&beginningSymbol);
	rc = e_shm_alloc(&neighborsExternal,"neighbors",(2*edgesNumber+vertexNumber)*sizeof(mint));
	if(rc != E_OK){
		//e_shm_release("neighbors");
		//rc = e_shm_alloc(&neighborsExternal,"neighbors",(2*edgesNumber+vertexNumber)*sizeof(int));
		rc=e_shm_attach(&neighborsExternal,"neighbors");
		if(rc != E_OK){
			printf("Failed neighborsExternal memory access\n");
		}
	}

	neighbors=malloc((vertexNumber)*sizeof(mint*));

	for(unsigned i=0;i<edgesNumber;i++){
		fscanf(f,"%c",&beginningSymbol);
		if(beginningSymbol==edgeSymbol){
			mint firstVertex,secondVertex;
			fscanf(f,"%d",&firstVertex);
			fscanf(f,"%d",&secondVertex);
			firstVertex--;
			secondVertex--;
			if(!neighMatrix[firstVertex][secondVertex]){
				neighCount[firstVertex]++;
				neighCount[secondVertex]++;
			}
			neighMatrix[firstVertex][secondVertex]=1;
			neighMatrix[secondVertex][firstVertex]=1;
		}
		fscanf(f,"%c",&beginningSymbol);
	}
	
	rc = e_shm_alloc(&offsetsExternal,"offsets",vertexNumber*sizeof(unsigned));
	if(rc != E_OK){
		//e_shm_release("offsets");
		//rc = e_shm_alloc(&offsetsExternal,"offsets",vertexNumber*sizeof(int));
		rc=e_shm_attach(&offsetsExternal,"offsets");
		if(rc != E_OK){
			printf("Failed offsets memory access\n");
		}
	}

	unsigned cnt=0;
	for(mint i=0;i<vertexNumber;i++){
		offsets[i]=cnt;
		neighbors[i]=malloc((neighCount[i]+1)*sizeof(mint));
		neighbors[i][0]=neighCount[i];
		mint br=1;
		for(mint j=0;j<vertexNumber;j++){
			if(neighMatrix[i][j]){
				neighbors[i][br]=j;
				br++;
			}
		}
		
		e_write(&neighborsExternal,0,0,cnt*sizeof(mint),neighbors[i],sizeof(mint)*(neighCount[i]+1));
		cnt=cnt+neighCount[i]+1;
	}
	e_write(&offsetsExternal,0,0,0,offsets,vertexNumber*sizeof(unsigned));

	/*printf("Matrix\n");
	for(int i=0;i<vertexNumber;i++){
		printf("Vertex %d has neighbors:",i+1);
		for(int j=1;j<=neighbors[i][0];j++){
			printf(" %d", neighbors[i][j]);	
		}
		printf("\n");
	}*/
}

void initializeMemory(){
	
	rc = e_shm_alloc(&memoryExternal,"memory",vertexNumber*vertexNumber*sizeof(float));
	if(rc != E_OK){
		//e_shm_release("memory");
		//rc = e_shm_alloc(&memoryExternal,"memory",vertexNumber*vertexNumber*sizeof(float));
		rc=e_shm_attach(&memoryExternal,"memory");
		if(rc != E_OK){
			printf("Failed memory memory access\n");
		}
	}

	for(mint i=0; i<vertexNumber; i++){
		for(mint j=0;j<vertexNumber;j++){
			memory[i][j]=1.0;
		}
	}
}

void initializeDeltaMemory(){
	for(mint i=0; i<vertexNumber; i++){
		memset(deltaMemory[i],0,sizeof(float));
	}
}

mint answer;
e_epiphany_t dev,devAll;
e_platform_t platform;

void distributeTasks(){

	for(mint i=0;i<vertexNumber;i++){
		e_write(&memoryExternal,0,0,i*vertexNumber*sizeof(float),memory[i],sizeof(float)*vertexNumber);
	}

	for(mint i=0;i<platform.rows;i++){
		for(mint j=0;j<platform.cols;j++){

			mint coreID=e_get_num_from_coords(&dev,i,j);
			int coreSeed=time(NULL);
			if(coreSeed>0){
				coreSeed=-coreSeed;
			}
			if(coreSeed==0){
				coreSeed--;
			}
			e_write(&coresProcesses,0,0,coreID*sizeof(int),&coreSeed,sizeof(int));
		}
	}

	
	e_start_group(&dev);

	mint ready=0;
	mint cores=platform.rows*platform.cols;
	while(ready<cores){
		for(mint i=0;i<platform.rows;i++){
			for(mint j=0;j<platform.cols;j++){
				
				int coreID=e_get_num_from_coords(&dev,i,j);
				int coreReady;
				
				int val=e_read(&coresProcesses,0,0,coreID*sizeof(int),&coreReady,sizeof(int));
				if(val==E_ERR) printf("Reading error\n");

				if(coreReady>=0){

					ready++;

					val=e_read(&dev,i,j,coreReady,solution,(vertexNumber+1)*sizeof(mint));
					if(val == E_ERR) printf("Reading error\n");

					coreReady=-1;
					e_write(&coresProcesses,0,0,coreID*sizeof(int),&coreReady,sizeof(int));
					for(mint firstVertex=0;firstVertex<vertexNumber;firstVertex++){
						for(mint secondVertex=0;secondVertex<vertexNumber;secondVertex++){
							if(firstVertex!=secondVertex&&solution[firstVertex]==solution[secondVertex]){
								deltaMemory[firstVertex][secondVertex]+=1.0/solution[vertexNumber];
							}
						}
					}

					if(answer > solution[vertexNumber]){

						answer = solution[vertexNumber];
						printf("Solution found with %d colors from core %d.\n",solution[vertexNumber], coreID);
						for(mint i=0;i<vertexNumber;i++){
							finalSolution[i]=solution[i];
							printf("%d ",solution[i]);
						}
						printf("\n");
					}

				}
			}
		}
	}

	for(mint vertex1=0; vertex1<vertexNumber; vertex1++){
		for(mint vertex2=0; vertex2<vertexNumber; vertex2++){
			if(neighMatrix[vertex1][vertex2] == 0){
				memory[vertex1][vertex2] = evaporationRate * memory[vertex1][vertex2] + deltaMemory[vertex1][vertex2];
			}
		}		
	}

}

void destructor(){
	
	e_shm_release("vertexNumber");
	e_shm_release("offsets");
	e_shm_release("neighbors");
	e_shm_release("memory");
	e_shm_release("messages");
	e_shm_release("cores");
	printf("Released memory\n");
	for(mint i=0;i<vertexNumber;i++){
		free(neighbors[i]);
	}
	free(neighbors);
}

int main(int argc, char** argv){

	//loader and host verbosity
	e_set_loader_verbosity(H_D0);
	e_set_host_verbosity(H_D0);	
	
	printf("Started host main\n");
	e_init(NULL);
	e_reset_system();
	
	input(argv[1]);

	rc = e_shm_alloc(&messages,"message",10024*sizeof(char));
	if(rc != E_OK){
		//e_shm_release("message");
		//rc = e_shm_alloc(&messages,"message",10024*sizeof(char));
		rc = e_shm_attach(&messages,"message");
		if(rc != E_OK){
			printf("Failed message memory access\n");
		}
	}

	e_get_platform_info(&platform);
	rc = e_shm_alloc(&coresProcesses,"cores",platform.rows*platform.cols*sizeof(int));
	if(rc != E_OK){
		//e_shm_release("cores");
		//rc = e_shm_alloc(&coresProcesses,"cores",platform.rows*platform.cols*sizeof(int));
		rc = e_shm_attach(&coresProcesses,"cores");
		if(rc != E_OK){
			printf("Failed cores memory access\n");
		}
	}


	printf("Finished input, initializing memory\n");
	initializeMemory();
	
	printf("Initializing epiphany\n");
	e_open(&dev, 0, 0, platform.rows, platform.cols);
	e_reset_group(&dev);
	e_load_group("bin/node.elf", &dev, 0, 0, platform.rows, platform.cols, E_FALSE);

	unsigned numberOfCycles=cyclesMultiplier*vertexNumber;
	answer=vertexNumber;
	
	printf("Starting cycles\n");
	for(unsigned i=0; i<numberOfCycles; i++){
		if(i%10==0){
			printf("Cycle %d from %d\n",i,numberOfCycles);
		}

		initializeDeltaMemory();
		distributeTasks();
	}
	
	e_close(&dev);

	FILE *f=fopen(argv[2],"w");

	printf("The answer is %d\n",answer);
	for(mint i=0;i<vertexNumber;i++){
		printf("%d ",finalSolution[i]);
		fprintf(f,"%d ",finalSolution[i]);
	}
	fprintf(f,"%d\n",answer);
	printf("\n");
	destructor();

	e_finalize();
	return 0;
}

