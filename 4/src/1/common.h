#include <iostream>
#include <fstream>
#include <sstream>
#include <mpi.h>

constexpr int N = 4000;
constexpr int NUM_ITERS = 100;

bool read_file(double **, char *);