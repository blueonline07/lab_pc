#!/bin/zsh
g++-15 -fopenmp src/common.cpp src/common.h src/openmp.cpp -o openmp
./openmp input/heat_matrix.csv