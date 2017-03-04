INCLUDEDIR=-Iinclude/
CFLAGS= ${INCLUDEDIR} -O2 --std=c++11

all: bin input output antcol genetic simann tabus

bin:
	@mkdir -p bin

input: bin src/io/input.cpp
	@echo "CC $<"
	@mpicxx ${CFLAGS} src/io/input.cpp -c -o bin/input.o

output: bin src/io/output.cpp
	@echo "CC $<"
	@mpicxx ${CFLAGS} src/io/output.cpp -c -o bin/output.o

antcol: bin input output
	@echo "Compiling ANTCOL"
	@mpicxx ${CFLAGS} -Iinclude/antcol/ src/antcol/master/master.cpp -c -o bin/antcolMaster.o
	@mpicxx ${CFLAGS} -Iinclude/antcol/ src/antcol/node/node.cpp -c -o bin/antcolNode.o
	@mpicxx ${CFLAGS} -Iinclude/antcol/ src/antcol/main.cpp -c -o bin/antcolMain.o
	@ld -r bin/antcolMain.o bin/antcolNode.o bin/antcolMaster.o -o bin/antcol.o
	@mpicxx ${CFLAGS} -o bin/antcol bin/input.o bin/output.o bin/antcol.o

genetic: bin input output
	@echo "Compiling Genetic Algorithm"
	@mpicxx ${CFLAGS} -Iinclude/genetic/ src/genetic/Communication/Communication.cpp -c -o bin/geneticCommunication.o 
	@mpicxx ${CFLAGS} -Iinclude/genetic/ src/genetic/Genotype/Genotype.cpp -c -o bin/geneticGenotype.o
	@mpicxx ${CFLAGS} -Iinclude/genetic/ src/genetic/master/master.cpp -c -o bin/geneticMaster.o
	@mpicxx ${CFLAGS} -Iinclude/genetic/ src/genetic/node/node.cpp -c -o bin/geneticNode.o
	@mpicxx ${CFLAGS} -Iinclude/genetic/ src/genetic/main.cpp -c -o bin/geneticMain.o
	@ld -r bin/geneticMain.o bin/geneticNode.o bin/geneticMaster.o bin/geneticGenotype.o bin/geneticCommunication.o -o bin/genetic.o
	@mpicxx ${CFLAGS} -o bin/genetic bin/input.o bin/output.o bin/genetic.o

simann: bin input
	@echo "Compiling Simulated Annealing"
	@mpicxx ${CFLAGS} src/simann/main.cpp -c -o bin/simann.o
	@mpicxx ${CFLAGS} -o bin/simann bin/input.o bin/simann.o

tabus: bin input
	@echo "Compiling Tabu Search"
	@mpicxx ${CFLAGS} src/tabus/main.cpp -c -o bin/tabus.o
	@mpicxx ${CFLAGS} -o bin/tabus bin/input.o bin/tabus.o

clean:
	@echo "Cleaning binaries"
	@rm -rf bin/
