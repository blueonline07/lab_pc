#!/bin/zsh
mpicxx ./src/parallel.cpp -o parallel
mpirun -np $NPROC ./parallel ./input/radioactive_matrix.csv