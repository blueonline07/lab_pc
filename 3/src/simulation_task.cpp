#include "simulation_task.h"

void SimulationTask::execute() {
    for (int row = start_row; row < end_row; row++) {
        for (int col = 0; col < map_size; col++) {
            // Calculate distance from center
            double distance = PhysicsUtils::calculateDistance(row, col, map_size, cell_size);
            
            // Check if shock wave has reached this cell
            if (PhysicsUtils::hasShockReached(distance, time_step)) {
                // Calculate and store peak overpressure
                double overpressure = PhysicsUtils::calculatePeakOverpressure(distance, yield);
                map[row * map_size + col] = overpressure;
            }
        }
    }
}
