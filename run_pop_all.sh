#!/bin/bash

make build_parallel

if [ $? -ne 0 ]; then
    echo "Compilation failed ☠️"
    exit 1
fi

echo

run () {
    echo "C: $1 N: $2"
    salloc -p pops -N 8 srun -n 1 time -f "%e" mpirun -c $1 -npernode $2 ./prac_p > /dev/null
    echo "--"
}


run 2 1
run 4 1
run 8 1
run 4 2
run 8 2
run 16 2
run 12 3
run 24 3
run 32 4
run 64 8
run 128 16
