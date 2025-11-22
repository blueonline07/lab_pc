#include "simulation.h"
#include "arg_parser.h"
#include <chrono>
#include <iostream>
#include <iomanip>

int main(int argc, char* argv[])
{
    ProgramOptions opts = parse_arguments(argc, argv, argv[0], false);
    
    ContaminationSimulation simulation(GRID_SIZE, GRID_SIZE);
    simulation.initializeFromFile(opts.input_file);

    auto t0 = std::chrono::high_resolution_clock::now();
    for (int step = 0; step < SIMULATION_TIME; ++step)
    {
        simulation.simulateStep();
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    double secs = std::chrono::duration<double>(t1 - t0).count();

    std::cout << std::fixed << std::setprecision(3)
              << "Sequential: " << secs << " s\n";
    
    if (opts.save_output) {
        simulation.saveToCSV("lab2_sequential_result.csv");
    } else {
        std::cout << "CSV output skipped.\n";
    }
    
    return 0;
}
