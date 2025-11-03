#ifndef SIMULATION_TASK_H
#define SIMULATION_TASK_H

#include "task.h"
#include "physics_utils.h"

class SimulationTask : public Task {
private:
    int time_step;
    int start_row;
    int end_row;
    double* map;
    int map_size;
    double yield;
    double cell_size;
    
public:
    SimulationTask(int id, int time_step, int start_row, int end_row, 
                   double* map, int map_size, double yield, double cell_size)
        : Task(id), time_step(time_step), start_row(start_row), end_row(end_row),
          map(map), map_size(map_size), yield(yield), cell_size(cell_size) {}
    
    void execute() override;
};

#endif // SIMULATION_TASK_H
