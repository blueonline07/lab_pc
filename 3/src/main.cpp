#include <iostream>
#include "shock_wave_simulation.h"

int main() {
    std::cout << "Nuclear Blast Shock Wave Simulation" << std::endl;
    std::cout << "===================================" << std::endl;
    
    const int NUM_THREADS = 4;
    
    // Create simulation instance
    ShockWaveSimulation simulation(NUM_THREADS);
    
    std::cout << "\nRunning Sequential Simulation..." << std::endl;
    simulation.runSequentialSimulation();
    simulation.printResults();
    
    std::cout << "\nRunning Parallel Simulation..." << std::endl;
    simulation.resetMap(); // Reset for parallel run
    simulation.runParallelSimulation();
    simulation.printResults();
    
    std::cout << "\nSimulation completed successfully!" << std::endl;
    
    return 0;
}
