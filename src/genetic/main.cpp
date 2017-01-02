#include "input.h"
#include "Communication.h"
#include "master.h"
#include "node.h"

int main(int argc,char **argv){

	if(argc!=3){
		printf("The application accepts two files as input and output as parameters.\n Example: mpirun -n N bin/genetic input.txt answer.txt\n");
		return 0;
	}
	readInput(argv[1]);

	communication::init(vertexNumber);

	if(communication::master()){
		masterMain(argv[2]);
	}else{
		if(communication::GM()){
			GMMain();
		}else{
			nodeMain();
		}
	}

	communication::finish();
	return 0;
}
