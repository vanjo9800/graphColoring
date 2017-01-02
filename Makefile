INCLUDEDIR=-Iinclude/
CFLAGS= ${INCLUDEDIR} -O2 --std=c++11

all: bin input output bin/antcol bin/genetic bin/simann

bin:
	@mkdir -p bin

input: src/io/input.cpp
	@echo "CC $<"
	@mpicxx ${CFLAGS} src/io/input.cpp -c -o bin/input.o

output: src/io/output.cpp
	@echo "CC $<"
	@mpicxx ${CFLAGS} src/io/output.cpp -c -o bin/output.o

bin/antcol: input output
	@echo "Compiling ANTCOL"
	@mpicxx ${CFLAGS} -Iinclude/antcol/ src/antcol/master/master.cpp -c -o bin/antcolMaster.o
	@mpicxx ${CFLAGS} -Iinclude/antcol/ src/antcol/node/node.cpp -c -o bin/antcolNode.o
	@mpicxx ${CFLAGS} -Iinclude/antcol/ src/antcol/main.cpp -c -o bin/antcolMain.o
	@ld -r bin/antcolMain.o bin/antcolNode.o bin/antcolMaster.o -o bin/antcol.o
	@mpicxx ${CFLAGS} -o bin/antcol bin/input.o bin/output.o bin/antcol.o

bin/genetic: input output
	@echo "Compiling Genetic Algorithm"
	@mpicxx ${CFLAGS} -Iinclude/genetic/ src/genetic/Communication/Communication.cpp -c -o bin/geneticCommunication.o 
	@mpicxx ${CFLAGS} -Iinclude/genetic/ src/genetic/Genotype/Genotype.cpp -c -o bin/geneticGenotype.o
	@mpicxx ${CFLAGS} -Iinclude/genetic/ src/genetic/master/master.cpp -c -o bin/geneticMaster.o
	@mpicxx ${CFLAGS} -Iinclude/genetic/ src/genetic/node/node.cpp -c -o bin/geneticNode.o
	@mpicxx ${CFLAGS} -Iinclude/genetic/ src/genetic/main.cpp -c -o bin/geneticMain.o
	@ld -r bin/geneticMain.o bin/geneticNode.o bin/geneticMaster.o bin/geneticGenotype.o bin/geneticCommunication.o -o bin/genetic.o
	@mpicxx ${CFLAGS} -o bin/genetic bin/input.o bin/output.o bin/genetic.o

bin/simann: input
	@echo "Compiling Simulated Annealing"
	@mpicxx ${CFLAGS} src/simann/main.cpp -c -o src/simann/simann.o
	@mpicxx ${CFLAGS} -o bin/simann bin/input.o src/simann/simann.o

clean:
	@echo "Cleaning binaries"
	@rm -rf bin/
