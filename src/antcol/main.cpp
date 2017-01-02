#include "input.h"
#include "master.h"
#include "node.h"

int main(int argc,char **argv){
	if(argc!=3){
		printf("The application accepts two files as input and output as parameters.\n Example: mpirun -n N bin/antcol input.txt answer.txt\n");
		return 0;
	}
	readInput(argv[1]);

	MPI_Init(NULL,NULL);
	int myRank;
	MPI_Comm_rank(MPI_COMM_WORLD,&myRank);

	if(myRank==0){
		masterMain(argv[2]);
	}else{
		nodeMain();
	}

	MPI_Finalize();
	return 0;
}
