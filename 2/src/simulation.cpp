#include "simulation.h"

void simulate_sequential(double **grid) {
    auto start_time = std::chrono::high_resolution_clock::now();
    for(int t = 0; t < SIMULATION_STEPS; t++){
        for(int i = 1; i <= GRID_SIZE; i++){
            for(int j = 1; j <= GRID_SIZE; j++){
                double advection = WIND_X * (grid[i][j] - grid[i-1][j]) / DX + WIND_Y * (grid[i][j] - grid[i][j-1]) / DY;
                double diffusion = DIFFUSION_COEFF * (grid[i+1][j] - 2*grid[i][j] + grid[i-1][j]) / (DX*DX) + DIFFUSION_COEFF * (grid[i][j+1] - 2*grid[i][j] + grid[i][j-1]) / (DY*DY);
                double decay = DECAY_RATE * grid[i][j] + DEPOSITION_RATE * grid[i][j];
                grid[i][j] = grid[i][j] + TIME_STEP * (-advection + diffusion - decay);
                grid[i][j] = std::max(0.0, grid[i][j]);
            }
        }
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Sequential: " << duration << " ms" << std::endl;
}


void simulate_parallel(double **grid){
    auto start_time = std::chrono::high_resolution_clock::now();

    auto end_time = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Parallel: " << duration << " ms" << std::endl;
}