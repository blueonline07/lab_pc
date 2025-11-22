#ifndef SHOCK_WAVE_SIMULATION_H
#define SHOCK_WAVE_SIMULATION_H

#include <vector>
#include <memory>
#include <chrono>
#include "worker.h"
#include "simulation_task.h"
#include "task_queue.h"

class ShockWaveSimulation
{
private:
    static const int MAP_SIZE = 4000;
    static const int TIME_STEPS = 100;
    static constexpr double YIELD = 5000.0;
    static constexpr double CELL_SIZE = 10.0;

    double *map;
    std::vector<std::unique_ptr<Worker>> workers;
    int num_threads;

public:
    ShockWaveSimulation(int num_threads = 4) : num_threads(num_threads)
    {
        map = new double[MAP_SIZE * MAP_SIZE];
        std::fill(map, map + MAP_SIZE * MAP_SIZE, 0.0);

        for (int i = 0; i < num_threads; i++)
        {
            workers.push_back(std::make_unique<Worker>(i));
        }
    }

    ~ShockWaveSimulation()
    {
        TaskQueue::get()->signalShutdown();
        
        for (auto &worker : workers)
        {
            worker->exit();
        }
        workers.clear();
        delete[] map;
    }

    void runParallelSimulation(bool save_output = true);
    void runSequentialSimulation(bool save_output = true);
    void saveToCSV(const char* filename) const;

    double *getMap() { return map; }
    int getMapSize() const { return MAP_SIZE; }

    void resetMap();

private:
    void createTasksForTimeStep(int time_step);
    void waitForCompletion();
};

#endif
