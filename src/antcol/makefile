EXECS=antcol.o
DEPENDENCIES=master node main
CLEANTEMP=cleanTemporary
MPICXX=mpicxx
INCLUDEDIR=-I../../include/antcol -I../../include
CFLAGS= ${INCLUDEDIR} -O2 --std=c++11

.PHONY: all master node main
all: ${EXECS}

master: master/master.cpp
	${MPICXX} ${CFLAGS} master/master.cpp -c -o master.o

node: node/node.cpp
	${MPICXX} ${CFLAGS} node/node.cpp -c -o node.o

main: main.cpp
	${MPICXX} ${CFLAGS} main.cpp -c -o main.o

antcol.o: ${DEPENDENCIES} main.o master.o node.o
	ld -r master.o node.o main.o -o antcol.o

clean:
	rm -rf *.o
