#ifndef __INPUT_INCLUDED__
#define __INPUT_INCLUDED__

#include<vector>
#include<fstream>
#include<string>
#include<sstream>
#include<limits>

const int MaxVertexNumber = 1001;
extern const char* inputFileName;
extern const char commentSymbol;
extern const char graphNodesSymbol;
extern const char edgeSymbol;
 
extern int vertexNumber;
extern int edgesNumber;
extern std::vector<int> neighbors[];

void readInput(char *inputFile);

#endif 
