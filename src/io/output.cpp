#include "output.h"

void output(char *outputFile,int answerSize,int* answer){

	std::ofstream out(outputFile);
	out<<answerSize<<" ";
	printf("\n\nColors used: %d\n",answerSize);
	for(int i=0;i<vertexNumber;i++){
		out<<answer[i]<<" ";
		printf("%d ",answer[i]);
	}
	out<<'\n';
	printf("\n");
	delete answer;
}
