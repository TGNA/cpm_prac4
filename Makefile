CC = gcc
SRC_S = sequential.c
BIN_S = prac_s
SRC_P = parallel.c
BIN_P = prac_p
CFLAGS = -Wall -g -O -fopenmp -lm

all : build

build: build_sequential build_parallel

build_sequential:
	$(CC) -o $(BIN_S) $(SRC_S) $(CFLAGS)

build_parallel:
	$(CC) -o $(BIN_P) $(SRC_P) $(CFLAGS)

clean : 
	rm -f *~ $(BIN_S) $(BIN_P) output/*