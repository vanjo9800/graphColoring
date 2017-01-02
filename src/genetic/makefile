EXECS=genetic.o
DEPENDENCIES=Communication Genotype output master node main
CLEANTEMP=cleanTemporary
MPICXX=mpicxx
INCLUDEDIR=-I../../include/genetic -I../../include
CFLAGS= ${INCLUDEDIR} -O2 --std=c++11

.PHONY: all Communication Genotype output master node main
all: ${EXECS}

Communication: Communication/Communication.cpp
	${MPICXX} ${CFLAGS} Communication/Communication.cpp -c -o Communication.o 

Genotype: Genotype/Genotype.cpp
	${MPICXX} ${CFLAGS} Genotype/Genotype.cpp -c -o Genotype.o

master: master/master.cpp
	${MPICXX} ${CFLAGS} master/master.cpp -c -o master.o

node: node/node.cpp
	${MPICXX} ${CFLAGS} node/node.cpp -c -o node.o

main: main.cpp
	${MPICXX} ${CFLAGS} main.cpp -c -o main.o

genetic.o: ${DEPENDENCIES} main.o Communication.o Genotype.o master.o node.o
	ld -r Communication.o Genotype.o master.o node.o main.o -o genetic.o

clean:
	rm -rf *.o
