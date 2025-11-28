#ifndef SIMULATION_H
#define SIMULATION_H

#include <fstream>
#include <sstream>
#include <chrono>
#include <iostream>
#include <mpi.h>
#include <vector>
#include <cstring>

constexpr int MPI_SIZE = 4;

constexpr int GRID_SIZE = 4000;
constexpr double DX = 10.0;
constexpr double DY = 10.0;
constexpr int SIMULATION_TIME = 100;
constexpr int TIME_STEP = 1;
constexpr int SIMULATION_STEPS = SIMULATION_TIME / TIME_STEP;

constexpr double DIFFUSION_COEFF = 1000.0;
constexpr double DECAY_RATE = 3e-5;
constexpr double DEPOSITION_RATE = 1e-4;
constexpr double WIND_X = 3.3;
constexpr double WIND_Y = 1.4;

constexpr double INITIAL_CONTAMINATION = 1000.0;
constexpr int INITIAL_X = GRID_SIZE / 2;
constexpr int INITIAL_Y = GRID_SIZE / 2;

constexpr double CONTAMINATION_THRESHOLD = 1e-10;

void simulate_sequential(double **);
void simulate_parallel(double **);
#endif
