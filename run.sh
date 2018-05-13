#!/bin/bash

# MPI ARGUMENTS
NUM_PROC=2
P_NODE=2

# CHECK OS
unamestr=`uname`
if [[ "$unamestr" == 'Darwin' ]]; then
   TIME='gtime'
else
   TIME='time'
fi

# SCRIPT
make clean

make build

if [ $? -ne 0 ]; then
    echo "Compilation failed ☠️"
    exit 1
fi

echo "Running sequential..."
TSEQ=$( { $TIME -f "%e" ./prac_s > ./output/sequential.txt; } 2>&1 )
echo "Time: $TSEQ"

echo "Running parallel..."
TPAR=$( { $TIME -f "%e" mpirun -c $NUM_PROC -npernode $P_NODE ./prac_p > ./output/parallel.txt; } 2>&1 )
echo "Time: $TPAR"

diff ./output/sequential.txt ./output/parallel.txt > /dev/null

if [ $? -eq 0 ]; then
    echo "OK 👍"
else
    echo "FAIL ☠️"
    exit 1
fi

SPEEDUP=$(echo "$TSEQ / $TPAR" | bc -l)
echo "Speedup: $SPEEDUP"
