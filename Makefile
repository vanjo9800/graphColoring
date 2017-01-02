INCLUDEDIR=-Iinclude/
CFLAGS= ${INCLUDEDIR} -O2 --std=c++11


all: bin input output bin/antcol bin/genetic bin/simann

bin:
	@mkdir -p bin

input: src/input/input.cpp
	@echo "CC $<"
	@mpicxx ${CFLAGS} src/input/input.cpp -c -o bin/input.o

output: src/output/output.cpp
	@echo "CC $<"
	@mpicxx ${CFLAGS} src/output/output.cpp -c -o bin/output.o

bin/antcol: input output
	@echo "Compiling ANTCOL"
	@cd src/antcol; make
	@mpicxx ${CFLAGS} -o bin/antcol bin/input.o bin/output.o src/antcol/antcol.o

bin/genetic: input output
	@echo "Compiling Genetic Algorithm"
	@cd src/genetic; make
	@mpicxx ${CFLAGS} -o bin/genetic bin/input.o bin/output.o src/genetic/genetic.o

bin/simann: input output
	@echo "Compiling Simulated Annealing"
	@mpicxx ${CFLAGS} src/simann/main.cpp -c -o src/simann/simann.o
	@mpicxx ${CFLAGS} -o bin/simann bin/input.o src/simann/simann.o

cleanTemporary:
	@echo "Cleaning temporary files"
	@cd src/antcol; make clean;
	@cd ../genetic; make clean;
	@rm -rf src/input/input.o
	@rm -rf src/output/ouput.o

clean: cleanTemporary
	@echo "Cleaning binaries"
	@rm -rf bin/
