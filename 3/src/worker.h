#ifndef WORKER_H
#define WORKER_H

#include <thread>
#include <atomic>
#include "task_queue.h"

class Worker {
private:
    std::atomic<bool> stop;
    std::thread t;
    int id;

    void run() {
        while (!stop.load()) {
            Task* task = TaskQueue::get()->dequeue();
            if (task != nullptr) {
                task->execute();
                TaskQueue::get()->markComplete();
                delete task;
            } else {
                break;
            }
        }
    }

public:
    Worker(int id) : stop(false), id(id) {
        t = std::thread(&Worker::run, this);
    }

    ~Worker() {
        exit();
    }

    void exit() {
        stop = true;
        if (t.joinable()) {
            t.join();
        }
    }

    int getId() const { return id; }
};

#endif
