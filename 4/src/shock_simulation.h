#ifndef SHOCK_SIMULATION_H
#define SHOCK_SIMULATION_H

#include <mpi.h>
#include "physics_utils.h"

class ShockSimulation {
private:
    static const int MAP_SIZE = 4000;
    static const int TIME_STEPS = 100;
    static constexpr double YIELD = 5000.0;   // kilotons
    static constexpr double CELL_SIZE = 10.0; // meters
    
    double *local_map;
    double *global_map;
    int rank;
    int size;
    int local_rows;
    int start_row;
    int end_row;
    
    void computeLocalRegion(int time_step);
    
public:
    ShockSimulation(int rank, int size);
    ~ShockSimulation();
    
    // Synchronous implementation (barrier after each time step)
    double runSynchronous();
    
    // Asynchronous implementation (non-blocking communication, overlap computation)
    double runAsynchronous();
    
    // Save results to CSV (only on rank 0)
    void saveToCSV(const char* filename);
    
    // Get map size
    int getMapSize() const { return MAP_SIZE; }
};

#endif // SHOCK_SIMULATION_H

