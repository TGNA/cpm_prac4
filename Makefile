CC_S = gcc
SRC_S = sequential.c
BIN_S = prac_s
CFLAGS_S = -Wall -O -lm

CC_P = mpicc
SRC_P = parallel.c
BIN_P = prac_p
CFLAGS_P = -Wall -O -lm

all : build

build: build_sequential build_parallel

build_sequential:
	$(CC_S) -o $(BIN_S) $(SRC_S) $(CFLAGS_S)

build_parallel:
	$(CC_P) -o $(BIN_P) $(SRC_P) $(CFLAGS_P)

clean : 
	rm -f *~ $(BIN_S) $(BIN_P) output/*