#!/bin/bash

make build_parallel

if [ $? -ne 0 ]; then
    echo "Compilation failed ☠️"
    exit 1
fi

echo

run () {
    TPAR=$( { salloc -p pops -N 8 srun -n 1 time -f "%e" mpirun -c $2 -npernode $3 ./prac_p $1 > /dev/null; } 2>&1 )
    echo "D: $1 C: $2 N: $3 Time: $TPAR"
}

run_configurations() {
    run $1 2 1
    run $1 4 1
    run $1 8 1
    run $1 4 2
    run $1 8 2
    run $1 16 2
    run $1 12 3
    run $1 24 3
    run $1 32 4
    run $1 64 8
    run $1 128 16
}

run_configurations 2000
run_configurations 3000
run_configurations 4000
