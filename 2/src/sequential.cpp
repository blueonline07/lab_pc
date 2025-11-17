#include "simulation.h"
#include <chrono>
#include <iostream>
#include <iomanip>

int main()
{
    ContaminationSimulation simulation(GRID_SIZE, GRID_SIZE);
    simulation.initialize();

    auto t0 = std::chrono::high_resolution_clock::now();
    for (int step = 0; step < SIMULATION_TIME; ++step)
    {
        simulation.simulateStep();
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    double secs = std::chrono::duration<double>(t1 - t0).count();

    std::cout << std::fixed << std::setprecision(3)
              << "Sequential: " << secs << " s\n";
    return 0;
}
