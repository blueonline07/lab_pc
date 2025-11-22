#include <iostream>
#include "shock_wave_simulation.h"
#include "arg_parser.h"

int main(int argc, char* argv[])
{
    ProgramOptions opts = parse_arguments(argc, argv, argv[0], false);
    
    const int NUM_THREADS = 4;
    ShockWaveSimulation simulation(NUM_THREADS);

    simulation.runSequentialSimulation(opts.save_output);
    simulation.resetMap();
    simulation.runParallelSimulation(opts.save_output);

    return 0;
}
