#!/bin/zsh
g++-15 -fopenmp src/common.cpp src/common.h src/tiled.cpp -o tiled
./tiled input/heat_matrix.csv