#include "simulation.h"
#include "utils.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "=== Sequential Contamination Diffusion Simulation ===" << std::endl;
    std::cout << "Grid Size: " << GRID_SIZE << "x" << GRID_SIZE << std::endl;
    std::cout << "Simulation Time: " << SIMULATION_TIME << " seconds" << std::endl;
    std::cout << "Wind Velocity: (" << WIND_X << ", " << WIND_Y << ") m/s" << std::endl;
    std::cout << "Diffusion Coefficient: " << DIFFUSION_COEFF << std::endl;
    std::cout << "Decay Rate: " << DECAY_RATE << std::endl;
    std::cout << "Deposition Rate: " << DEPOSITION_RATE << std::endl;
    std::cout << std::endl;
    
    // Create simulation
    ContaminationSimulation simulation(GRID_SIZE, GRID_SIZE);
    
    // Initialize simulation
    simulation.initialize();
    
    std::cout << "Initial contamination at center: " << INITIAL_CONTAMINATION << std::endl;
    std::cout << "Initial uncontaminated blocks: " << simulation.countUncontaminatedBlocks() << std::endl;
    std::cout << std::endl;
    
    // Start timing
    SimulationUtils::Timer timer("Sequential Simulation");
    
    // Run simulation
    for (int step = 0; step < SIMULATION_TIME; step++) {
        simulation.simulateStep();
        
        // Report progress every 10 steps
        if (step % 10 == 0 || step == SIMULATION_TIME - 1) {
            std::cout << "Time Step " << std::setw(3) << step 
                      << ": Uncontaminated blocks = " 
                      << std::setw(8) << simulation.countUncontaminatedBlocks()
                      << ", Total contamination = " 
                      << std::fixed << std::setprecision(2) 
                      << simulation.getTotalContamination() << std::endl;
        }
    }
    
    // Print final results
    simulation.printResults();
    
    std::cout << "\nSequential simulation completed successfully!" << std::endl;
    
    return 0;
}
