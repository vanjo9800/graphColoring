#include "output.h"

void output(char *outputFile,int answerSize,int* answer){

	std::ofstream out(outputFile);
	out<<"Colors used: "<<answerSize<<"\n";
	printf("\n\nColors used: %d\n",answerSize);
	for(int i=0;i<vertexNumber;i++){
		out<<answer[i];
		printf("%d ",answer[i]);
	}
	printf("\n");
	printf("Press any key to close the window...\n");
	char c;
	//scanf("%c",&c);
	delete answer;
	out<<'\n';
}
