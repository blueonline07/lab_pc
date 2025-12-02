## Nuclear Blast Shock Wave Simulation Report

## Introduction

This project implements a nuclear blast shock wave simulation and compares
sequential vs. parallel performance using a work-pool model. The simulation
computes peak overpressure across a large map as the shock front expands outward
from the detonation center.

## Implementation

The parallel implementation uses a thread pool model with a shared task queue. The algorithm parallelizes the computation by distributing time steps across multiple worker threads. Each time step is packaged as a task and enqueued for parallel processing. Worker threads continuously dequeue tasks and process the entire grid for their assigned time step. The task queue uses mutexes and condition variables to ensure thread-safe access, allowing multiple workers to process different time steps concurrently without race conditions.

### Parallel Algorithm (Thread Pool)

```cpp
// Main thread enqueues tasks
for (int t = 0; t < TIME; t++)
{
    taskQueue.enqueue(new Task(t, grid, c));
}

// Worker thread processes tasks
void run() {
    while (true) {
        Task *task = taskQueue->dequeue();
        if (task == nullptr) break;
        
        // Process entire grid for time step t
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                // Same computation logic as sequential
            }
        }
        delete task;
    }
}
```

## Performance

### Performance Metrics

```
Speedup    = T_seq / T_par
Efficiency = Speedup / Workers
```

### Performance Comparision

| Implementation | Workers | Time (s) | Speedup | Efficiency |
| -------------- | ------- | -------- | ------- | ---------- |
| Sequential     | 1       | 103      | 1.00×   | 100%       |
| Parallel       | 4       | 29       | 3.6×    | 90%        |
| Parallel       | 8       | 21       | 4.9×    | 61%        |

---

**Report Generated**: December 2025\
**Simulation Environment**: macOS 14, C++17, pthreads, 4 workers
