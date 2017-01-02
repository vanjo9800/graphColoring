ESDK=${EPIPHANY_HOME}
ELIBS=-L${ESDK}/tools/host/lib
EINCS=-I${ESDK}/tools/host/include
ELDF=${ESDK}/bsps/current/fast.ldf

all: antcol

antcol: bin bin/master bin/node.elf

bin:
	@mkdir -p bin

bin/master: src/master.c
	@echo "CC $<"
	@gcc -std=c99 src/master.c -o bin/master ${EINCS} ${ELIBS} -le-hal -le-loader -lpthread

bin/node.elf: src/node.c
	@echo "CC $<"
	@e-gcc -std=c99 -T ${ELDF} src/node.c -o bin/node.elf -le-lib

clean:
	@rm -r bin
