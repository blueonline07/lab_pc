## Nuclear Blast Shock Wave Simulation Report

## Introduction

This project implements a nuclear blast shock wave simulation and compares
sequential vs. parallel performance using a work-pool model. The simulation
computes peak overpressure across a large map as the shock front expands outward
from the detonation center.

## Implementation

Using threadpool model in Lab 3 instruction.

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
