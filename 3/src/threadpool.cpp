#include "common.h"
#include <chrono>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>

#define NUM_THREADS 4

int sq(int x)
{
    return x * x;
}

struct Task
{
    int t;
    double **grid;
    const double *c;

    Task(int time, double **g, const double *coeffs) : t(time), grid(g), c(coeffs) {}
};

class TaskQueue
{
private:
    std::mutex mtx;
    std::condition_variable cv;
    std::queue<Task *> queue;
    bool shutdown;

public:
    TaskQueue() : shutdown(false) {}

    void enqueue(Task *task)
    {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(task);
        cv.notify_one();
    }

    Task *dequeue()
    {
        std::unique_lock<std::mutex> lock(mtx);
        while (queue.empty() && !shutdown)
        {
            cv.wait(lock);
        }
        if (shutdown && queue.empty())
        {
            return nullptr;
        }
        Task *task = queue.front();
        queue.pop();
        return task;
    }

    void stop()
    {
        std::lock_guard<std::mutex> lock(mtx);
        shutdown = true;
        cv.notify_all();
    }

    bool is_empty()
    {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.empty();
    }
};

class Worker
{
private:
    std::thread t;
    TaskQueue *taskQueue;

    void run()
    {
        while (true)
        {
            Task *task = taskQueue->dequeue();
            if (task == nullptr)
            {
                break;
            }

            int t = task->t;
            double **grid = task->grid;
            const double *c = task->c;

            for (int i = 0; i < N; i++)
            {
                for (int j = 0; j < N; j++)
                {
                    double R = sqrt(sq(i - CENTER_X) + sq(j - CENTER_Y)) * CELL_SIZE;

                    if (t >= R / 343.0)
                    {
                        double Z = R * pow(W, -1.0 / 3.0);
                        double U = -0.21436 + 1.35034 * log10(Z);
                        double log10P = 0.0;
                        for (int k = 0; k < 9; k++)
                        {
                            log10P += c[k] * pow(U, k);
                        }
                        grid[i][j] = pow(10.0, log10P);
                    }
                }
            }

            delete task;
        }
    }

public:
    Worker(TaskQueue *tq) : taskQueue(tq)
    {
        t = std::thread(&Worker::run, this);
    }

    void join()
    {
        if (t.joinable())
        {
            t.join();
        }
    }
};

int main(int argc, char *argv[])
{
    const double c[9] = {2.611369, -1.690128, 0.00805, 0.336743, -0.005162, -0.080923, -0.004785, 0.007930, 0.000768};

    // Allocate grid
    double **grid = new double *[N];
    for (int i = 0; i < N; i++)
    {
        grid[i] = new double[N];
        for (int j = 0; j < N; j++)
        {
            grid[i][j] = 0.0;
        }
    }

    TaskQueue taskQueue;
    std::vector<Worker *> workers;

    for (int i = 0; i < NUM_THREADS; i++)
    {
        workers.push_back(new Worker(&taskQueue));
    }

    auto start = std::chrono::high_resolution_clock::now();

    for (int t = 0; t < TIME; t++)
    {
        taskQueue.enqueue(new Task(t, grid, c));
    }

    while (!taskQueue.is_empty())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    taskQueue.stop();
    for (Worker *worker : workers)
    {
        worker->join();
        delete worker;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << std::endl;

    for (int i = 0; i < N; i++)
    {
        delete[] grid[i];
    }
    delete[] grid;

    return 0;
}