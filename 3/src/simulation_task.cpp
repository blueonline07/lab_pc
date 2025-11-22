#include "simulation_task.h"

void SimulationTask::execute() {
    for (int row = start_row; row < end_row; row++) {
        for (int col = 0; col < map_size; col++) {
            double distance = PhysicsUtils::calculateDistance(row, col, map_size, cell_size);
            
            if (PhysicsUtils::hasShockReached(distance, time_step)) {
                double overpressure = PhysicsUtils::calculatePeakOverpressure(distance, yield);
                map[row * map_size + col] = overpressure;
            }
        }
    }
}
