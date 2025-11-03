## Nuclear Blast Shock Wave Simulation Report

## Introduction

This project implements a nuclear blast shock wave simulation and compares sequential vs. parallel performance using a work-pool model. The simulation computes peak overpressure across a large map as the shock front expands outward from the detonation center.

### Physical Model

The model uses Kingery–Bulmash empirical relationships:

```
Z = R · W^(−1/3)
u = −0.21436 + 1.35034 · log10(Z)
log10(Pso) = Σ_{i=0..8} c_i · u^i
t_arrival = R / 343
```

Where:

- `R` = distance from the blast center (m)
- `W` = yield (kg TNT equivalent) — here 5000 kt
- `Pso` = peak overpressure (kPa)
- `t_arrival` = simplified arrival time assuming speed of sound

### Simulation Parameters

| Parameter         | Value       | Description                 |
| ----------------- | ----------- | --------------------------- |
| Grid Size         | 4000×4000   | Map resolution              |
| Cell Size         | 10.0 m      | Spatial resolution per cell |
| Simulation Time   | 100 seconds | Total modeled duration      |
| Time Step         | 1 second    | Discrete step size          |
| Yield             | 5000 kt     | TNT equivalent              |
| Workers (default) | 4           | Work-pool threads           |

## Formula Implementation

All physics is centralized in `physics_utils.*` and reused by both implementations to ensure identical results.

### Sequential Algorithm

At each time step, iterate the full grid; for each cell: compute `R`, check `t ≥ t_arrival`, and write `Pso` if reached.

### Parallelization Strategy (Work Pool)

#### Work Distribution

- For each time step, the map is split into horizontal strips.
- We enqueue `4×workers` tasks for finer granularity and better load balance.
- Workers pull tasks from a shared `TaskQueue` protected by a `condition_variable`.

#### Synchronization

- Main thread blocks until the queue’s completed count equals total tasks.
- A `clear()` API allows safe reuse of the singleton queue between runs.

### Implementation Details

1. Contiguous `double` array for the grid to maximize cache locality.
2. Identical physics code-paths in sequential and parallel implementations.
3. Horizontal strip scheduling to reduce false sharing and improve prefetching.

## Performance

### Performance Metrics

```
Speedup    = T_seq / T_par
Efficiency = Speedup / Workers
```

### Performance Comparision

| Implementation | Workers | Time (s) | Speedup | Efficiency |
| -------------- | ------- | -------- | ------- | ---------- |
| Sequential     | 1       | 27.510   | 1.00×   | 100%       |
| Parallel       | 4       | 7.449    | 3.69×   | 92.3%      |
---
**Report Generated**: October 2025  
**Simulation Environment**: macOS 14, C++17, pthreads, 4 workers
