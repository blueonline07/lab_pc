#include "shock_wave_simulation.h"
#include "physics_utils.h"
#include <iostream>
#include <iomanip>
#include <thread>

void ShockWaveSimulation::runParallelSimulation()
{
    auto start_time = std::chrono::high_resolution_clock::now();

    // Clear task queue instead of resetting
    TaskQueue::get()->clear();

    // Create tasks for each time step
    for (int time_step = 0; time_step < TIME_STEPS; time_step++)
    {
        createTasksForTimeStep(time_step);
    }

    // Wait for all tasks to complete
    waitForCompletion();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "Parallel: " << std::fixed << std::setprecision(3)
              << (duration.count() / 1000.0) << " s\n";
}

void ShockWaveSimulation::runSequentialSimulation()
{
    auto start_time = std::chrono::high_resolution_clock::now();

    // Process each time step sequentially
    for (int time_step = 0; time_step < TIME_STEPS; time_step++)
    {
        // Process entire map for this time step
        for (int row = 0; row < MAP_SIZE; row++)
        {
            for (int col = 0; col < MAP_SIZE; col++)
            {
                // Calculate distance from center
                double distance = PhysicsUtils::calculateDistance(row, col, MAP_SIZE, CELL_SIZE);

                // Check if shock has reached this cell
                if (PhysicsUtils::hasShockReached(distance, time_step))
                {
                    // Calculate and store peak overpressure
                    double overpressure = PhysicsUtils::calculatePeakOverpressure(distance, YIELD);
                    map[row * MAP_SIZE + col] = overpressure;
                }
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "Sequential: " << std::fixed << std::setprecision(3)
              << (duration.count() / 1000.0) << " s\n";
}

void ShockWaveSimulation::createTasksForTimeStep(int time_step)
{
    TaskQueue *queue = TaskQueue::get();

    // Create more tasks than threads for better load balancing
    // Use 4x the number of threads for better granularity
    int tasks_per_time_step = num_threads * 4;
    int rows_per_task = MAP_SIZE / tasks_per_time_step;

    for (int i = 0; i < tasks_per_time_step; i++)
    {
        int start_row = i * rows_per_task;
        int end_row = (i == tasks_per_time_step - 1) ? MAP_SIZE : (i + 1) * rows_per_task;

        int task_id = time_step * tasks_per_time_step + i;
        queue->enqueue(new SimulationTask(task_id, time_step, start_row, end_row,
                                          map, MAP_SIZE, YIELD, CELL_SIZE));
    }
}

void ShockWaveSimulation::waitForCompletion()
{
    TaskQueue *queue = TaskQueue::get();

    while (!queue->isComplete())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void ShockWaveSimulation::resetMap()
{
    std::fill(map, map + MAP_SIZE * MAP_SIZE, 0.0);
}
