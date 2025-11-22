#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "task.h"

class TaskQueue {
private:
    std::mutex mtx;
    std::condition_variable cv;
    std::queue<Task*> queue;
    std::atomic<int> completed_tasks{0};
    std::atomic<int> total_tasks{0};
    std::atomic<bool> shutdown{false};
    static TaskQueue* instance;
    static std::mutex instance_mutex;
    
public:
    void enqueue(Task* task) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(task);
        total_tasks++;
        cv.notify_one();
    }

    Task* dequeue() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !queue.empty() || shutdown.load(); });
        
        if (!queue.empty()) {
            Task* task = queue.front();
            queue.pop();
            return task;
        }
        return nullptr; // shutdown signal
    }

    bool isComplete() const {
        return completed_tasks.load() == total_tasks.load();
    }

    void markComplete() {
        completed_tasks++;
    }

    int getTotalTasks() const {
        return total_tasks.load();
    }

    int getCompletedTasks() const {
        return completed_tasks.load();
    }

    void signalShutdown() {
        shutdown.store(true);
        cv.notify_all();
    }

    static TaskQueue* get() {
        if (instance == nullptr) {
            std::lock_guard<std::mutex> lock(instance_mutex);
            if (instance == nullptr) {
                instance = new TaskQueue();
            }
        }
        return instance;
    }

    void resetCounters() {
        completed_tasks.store(0);
        total_tasks.store(0);
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mtx);
        while (!queue.empty()) {
            delete queue.front();
            queue.pop();
        }
        completed_tasks.store(0);
        total_tasks.store(0);
        shutdown.store(false);
    }

    static void reset() {
        if (instance != nullptr) {
            instance->signalShutdown();
            delete instance;
            instance = nullptr;
        }
    }
};

#endif
