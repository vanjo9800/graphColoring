#include <e_lib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MaxVertexNumber 201
typedef unsigned short mint;

mint vertexNumber;
mint coloring[MaxVertexNumber+1],newColor[MaxVertexNumber+1];
char candidatesForColorCheck[MaxVertexNumber],cannotBeColoredCheck[MaxVertexNumber];
mint candidatesForColor[MaxVertexNumber],candidatesForColorSize;
mint neighbors[MaxVertexNumber],size;

//float memory[MaxVertexNumber][MaxVertexNumber];
float probabilities[MaxVertexNumber];

unsigned offsets[MaxVertexNumber];
e_memseg_t neighborsExternal,vertexNumberExternal,offsetsExternal,memoryExternal,messages,coresProcesses;
//char text[10024],cur[50];

void cacheNeighbors(mint i){
	//e_read(&neighborsExternal,&ans,0,0,(void*)((offsets[i]+j)*sizeof(int)),sizeof(int));
	e_dma_copy(&size,(void*) (neighborsExternal.ephy_base + offsets[i]*sizeof(mint)),sizeof(mint));
	e_dma_copy(neighbors,(void*) (neighborsExternal.ephy_base + offsets[i]*sizeof(mint)),sizeof(mint)*(size+1));
}

float memory(mint i,mint j){
	
	float ans;
	//e_read(&memoryExternal,&ans,0,0,(void*)((offsets[i]+j)*sizeof(float)),sizeof(float));
	e_dma_copy(&ans,(void*) (memoryExternal.ephy_base + (i*vertexNumber+j)*sizeof(float)),sizeof(float));
	return ans;
}

mint desirability(mint vertex,mint choosingDesirability){
	if(choosingDesirability==0){
		mint desirabilityOfVertex=0;
		for(mint i=1;i<=size;i++){
			desirabilityOfVertex+=cannotBeColoredCheck[neighbors[i]];
		}
		return desirabilityOfVertex;
	}
	if(choosingDesirability==1){
		mint counter=0;
		for(mint i=1;i<=size;i++){
			counter+=candidatesForColorCheck[neighbors[i]];
		}
		return candidatesForColorSize-counter;
	}
	mint desirabilityOfVertex=0;
	for(mint i=1;i<=size;i++){
		desirabilityOfVertex+=cannotBeColoredCheck[neighbors[i]]||candidatesForColorCheck[neighbors[i]];
	}
	return desirabilityOfVertex;
}

float trailFactor(mint vertex){
	if(newColor[0]==0){
		return 1.0;
	}else{
		float trailFactorForVertex=0.0;
		for(mint i=1;i<=newColor[0];i++){
			trailFactorForVertex+=memory(newColor[i],vertex);
		}
		return trailFactorForVertex/newColor[0];
	}
}

float probability(mint vertex,mint choosingDesirability){
	return desirability(vertex,choosingDesirability)*trailFactor(vertex);
}

void swap(mint *a, mint *b){
	(*a)=(*a)^(*b);
	(*b)=(*a)^(*b);
	(*a)=(*b)^(*a);
}

void recursiveLargestFirst(){
	
	mint choosingFirstVertex=rand()%2;
	mint choosingDesirability=rand()%3;
	
	mint numberOfColors=0;

	candidatesForColorSize=vertexNumber;
	for(mint i=0;i<vertexNumber;i++){
		candidatesForColorCheck[i]=1;
		candidatesForColor[i]=i;
	}

	mint numberOfColored=0;
	newColor[0]=0;

	while(numberOfColored<vertexNumber){

		numberOfColored++;
		numberOfColors++;
		newColor[0]=0;
		
		mint vertex=0,position=0;
		if(choosingFirstVertex==0){
			mint MaxVertexNumberertexDegree=0;
			for(mint i=0;i<candidatesForColorSize;i++){
				
				mint counter=0;
				cacheNeighbors(candidatesForColor[i]);
				for(mint j=1;j<=size;j++){
					counter+=candidatesForColorCheck[neighbors[j]];
				}
				if(counter>=MaxVertexNumberertexDegree){
					MaxVertexNumberertexDegree=counter;
					vertex=candidatesForColor[i];
					position=i;
				}
			}
		}
		else{
			position=rand()%candidatesForColorSize;
			vertex=candidatesForColor[position];
		}
		
	//	sprintf(cur,"Color %d, vertex %d\n",numberOfColors,vertex+1);
	//	strcat(text,cur);
	//	e_write(&messages,text,0,0,0,sizeof(text));
		
		newColor[newColor[0]+1]=vertex;
		newColor[0]++;
		
		while(1){

			mint counter=1;
			cacheNeighbors(vertex);
			for(mint i=1;i<=size;i++){
				counter+=candidatesForColorCheck[neighbors[i]];
			}

			if(candidatesForColorSize<=counter){
				break;
			}

			numberOfColored++;
			candidatesForColorCheck[vertex]=0;
			swap(&candidatesForColor[position],&candidatesForColor[candidatesForColorSize-1]);
			candidatesForColorSize--;

			for(mint i=1;i<=size;i++){
				cannotBeColoredCheck[neighbors[i]]|=candidatesForColorCheck[neighbors[i]];
				candidatesForColorCheck[neighbors[i]]=false;
			}
			for(mint i=0;i<candidatesForColorSize;i++){
				while(i<candidatesForColorSize&&candidatesForColorCheck[candidatesForColor[i]]==0){
					swap(&candidatesForColor[i],&candidatesForColor[candidatesForColorSize-1]);
					candidatesForColorSize--;
				}
			}

			float sumOfProbabilities=0.0;
			for(mint i=0;i<candidatesForColorSize;i++){
				
				probabilities[candidatesForColor[i]]=probability(candidatesForColor[i],choosingDesirability);
				sumOfProbabilities+=probabilities[candidatesForColor[i]];
			}

			float nextVertex=sumOfProbabilities*((float) rand()/(float) RAND_MAX),currentSum=0.0;
		
			for(mint i=0;i<candidatesForColorSize;i++){

				currentSum+=probabilities[candidatesForColor[i]];
				if(currentSum>=nextVertex){
					vertex=candidatesForColor[i];
					position=i;
					break;
				}
			}

			newColor[newColor[0]+1]=vertex;
			newColor[0]++;
		
		//	sprintf(cur,"Color %d, vertex %d\n",numberOfColors,vertex+1);
		//	strcat(text,cur);
		//	e_write(&messages,text,0,0,0,sizeof(text));
		
		}
	
		for(mint i=1;i<=newColor[0];i++){
			coloring[newColor[i]]=numberOfColors;
		}
		
		for(mint i=1;i<=size;i++){
			cannotBeColoredCheck[neighbors[i]]|=candidatesForColorCheck[neighbors[i]];
		}
		candidatesForColorSize=0;

		for(mint i=0;i<vertexNumber;i++){
			candidatesForColorCheck[i]=cannotBeColoredCheck[i];
			if(cannotBeColoredCheck[i]){
				candidatesForColor[candidatesForColorSize++]=i;
			}
			cannotBeColoredCheck[i]=0;
		}
	}
	coloring[vertexNumber]=numberOfColors;
}

int main(){

	//strcpy(text,"");
	//e_shm_attach(&messages, "message");
	
	e_shm_attach(&vertexNumberExternal, "vertexNumber");
	e_shm_attach(&coresProcesses, "cores");
	e_shm_attach(&offsetsExternal, "offsets");
	e_shm_attach(&neighborsExternal, "neighbors");
	e_shm_attach(&memoryExternal, "memory");
	
	int row,col;
	e_coords_from_coreid(e_get_coreid(),&row,&col);
	mint coreID=row*e_group_config.group_cols+col;

	//sprintf(cur,"Core %d run...\n",coreID);
	//strcat(text,cur);
	//e_write(&messages,text,0,0,0,sizeof(text));

	while(1){
		int seed;
		//e_read(&coresProcesses,&seed,0,0,0,sizeof(int));
		e_dma_copy(&seed,(void*) (coresProcesses.ephy_base+coreID*sizeof(int)),sizeof(int));
		seed=-seed;
		srand(seed);
		
		//e_read(&vertexNumberExternal,&vertexNumber,0,0,0,sizeof(int));
		e_dma_copy(&vertexNumber,(void*) vertexNumberExternal.ephy_base,sizeof(mint));
		
		//sprintf(cur,"Vertex number %d\n",vertexNumber);
		//strcat(text,cur);
		//e_write(&messages,text,0,0,0,sizeof(text));
		
		//e_read(&offsetsExternal,offsets,0,0,0,vertexNumber*sizeof(int));
		e_dma_copy(offsets,(void*) offsetsExternal.ephy_base, vertexNumber*sizeof(unsigned));
		
	//	sprintf(cur,"Running RLF\n");
	//	strcat(text,cur);
	//	e_write(&messages,text,0,0,0,sizeof(text));
		
		recursiveLargestFirst();

	//	sprintf(cur,"Finished algorithm\n");
	//	strcat(text,cur);
	//	e_write(&messages,text,0,0,0,sizeof(text));
		
	//	sprintf(cur,"Used: %d colors\n",coloring[vertexNumber]);
	//	strcat(text,cur);
	//	e_write(&messages,text,0,0,0,sizeof(text));

		int address=0,sz;
		char symbolAddress[15];
		sprintf(symbolAddress,"%p",coloring);
		sz=strlen(symbolAddress);
		for(mint i=0;i<4;i++){
			address*=16;
			if(symbolAddress[sz-4+i]>='0'&&symbolAddress[sz-4+i]<='9'){
				address+=(symbolAddress[sz-4+i]-'0');
			}
			if(symbolAddress[sz-4+i]>='a'&&symbolAddress[sz-4+i]<='f'){
				address+=10+(symbolAddress[sz-4+i]-'a');
			}
		}
		//e_write(&coresProcesses,&address,0,0,0,sizeof(int));
		e_dma_copy((void*) (coresProcesses.ephy_base+coreID*sizeof(int)),&address,sizeof(int));

		__asm__ __volatile__ ("idle");
	}
	return 0;
}
