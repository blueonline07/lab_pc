#include <iostream>
#include <vector>
#include <memory>
#include "task.h"
#include "task_queue.h"
#include "worker.h"

// Simple test task
class TestTask : public Task {
private:
    int value;
public:
    TestTask(int id, int value) : Task(id), value(value) {}
    
    void execute() override {
        std::cout << "Task " << getId() << " executed with value " << value << std::endl;
        // Simulate some work
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
};

int main() {
    std::cout << "Testing Basic Work Pool Implementation" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    const int NUM_THREADS = 4;
    const int NUM_TASKS = 20;
    
    // Create workers
    std::vector<std::unique_ptr<Worker>> workers;
    for (int i = 0; i < NUM_THREADS; i++) {
        workers.push_back(std::make_unique<Worker>(i));
    }
    
    // Create and enqueue tasks
    TaskQueue* queue = TaskQueue::get();
    for (int i = 0; i < NUM_TASKS; i++) {
        queue->enqueue(new TestTask(i, i * 10));
    }
    
    std::cout << "Created " << NUM_TASKS << " tasks with " << NUM_THREADS << " workers" << std::endl;
    
    // Wait for completion
    while (!queue->isComplete()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::cout << "Progress: " << queue->getCompletedTasks() << "/" << queue->getTotalTasks() << std::endl;
    }
    
    std::cout << "\nAll tasks completed!" << std::endl;
    
    return 0;
}
