#include <iostream>
#include "shock_wave_simulation.h"

int main()
{
    const int NUM_THREADS = 4;
    ShockWaveSimulation simulation(NUM_THREADS);

    simulation.runSequentialSimulation();

    simulation.resetMap();
    simulation.runParallelSimulation();

    return 0;
}
