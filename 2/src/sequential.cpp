#include "simulation.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <string>

int main(int argc, char* argv[])
{
    // Parse command line arguments
    bool save_output = true;
    const char* input_file = nullptr;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--no-output" || arg == "--skip-csv") {
            save_output = false;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [options] [input_csv]\n"
                      << "Options:\n"
                      << "  --no-output, --skip-csv    Skip CSV output generation\n"
                      << "  --help, -h                 Show this help message\n"
                      << "Arguments:\n"
                      << "  input_csv                  Load initial conditions from CSV file\n";
            return 0;
        } else if (arg[0] != '-') {
            input_file = argv[i];
        }
    }
    
    ContaminationSimulation simulation(GRID_SIZE, GRID_SIZE);
    
    // Initialize - from CSV if provided, otherwise generate default
    if (input_file) {
        // Load from CSV file provided as command line argument
        if (!simulation.loadFromCSV(input_file)) {
            std::cerr << "Failed to load input CSV. Using default initialization." << std::endl;
            simulation.initialize();
        }
    } else {
        // Use default initialization
        simulation.initialize();
    }

    auto t0 = std::chrono::high_resolution_clock::now();
    for (int step = 0; step < SIMULATION_TIME; ++step)
    {
        simulation.simulateStep();
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    double secs = std::chrono::duration<double>(t1 - t0).count();

    std::cout << std::fixed << std::setprecision(3)
              << "Sequential: " << secs << " s\n";
    
    // Save results to CSV (if enabled)
    if (save_output) {
        simulation.saveToCSV("lab2_sequential_result.csv");
    } else {
        std::cout << "CSV output skipped.\n";
    }
    
    return 0;
}
