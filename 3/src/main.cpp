#include <iostream>
#include <string>
#include "shock_wave_simulation.h"

int main(int argc, char* argv[])
{
    // Parse command line arguments
    bool save_output = true;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--no-output" || arg == "--skip-csv") {
            save_output = false;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [options]\n"
                      << "Options:\n"
                      << "  --no-output, --skip-csv    Skip CSV output generation\n"
                      << "  --help, -h                 Show this help message\n";
            return 0;
        }
    }
    
    const int NUM_THREADS = 4;
    ShockWaveSimulation simulation(NUM_THREADS);

    simulation.runSequentialSimulation();
    if (save_output) {
        simulation.saveToCSV("lab3_sequential_result.csv");
    } else {
        std::cout << "CSV output skipped for sequential.\n";
    }

    simulation.resetMap();
    simulation.runParallelSimulation();
    if (save_output) {
        simulation.saveToCSV("lab3_parallel_result.csv");
    } else {
        std::cout << "CSV output skipped for parallel.\n";
    }

    return 0;
}
