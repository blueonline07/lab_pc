#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include "task.h"
#include "task_queue.h"
#include "worker.h"

// Simple test task
class TestTask : public Task
{
private:
    int value;

public:
    TestTask(int id, int value) : Task(id), value(value) {}
    void execute() override
    {
        (void)value;
        // Simulate some work without logging spam
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
};

int main()
{
    std::cout << "Work Pool Smoke Test" << std::endl;
    const int NUM_THREADS = 4;
    const int NUM_TASKS = 20;

    std::vector<std::unique_ptr<Worker>> workers;
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        workers.push_back(std::make_unique<Worker>(i));
    }

    TaskQueue *queue = TaskQueue::get();
    for (int i = 0; i < NUM_TASKS; ++i)
    {
        queue->enqueue(new TestTask(i, i * 10));
    }

    while (!queue->isComplete())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }

    queue->signalShutdown();

    std::cout << "All tasks completed." << std::endl;
    return 0;
}

