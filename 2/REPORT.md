# Radioactive Dispersion Simulation Report

## Introduction

This project implements a radioactive dispersion simulation using finite
difference methods to solve the advection-diffusion-decay equation. The
simulation models how radioactive contamination spreads through the atmosphere
under wind conditions, accounting for advection, diffusion, decay, and
deposition processes.

## Parallelization Strategy

### Partition

```cpp
// Use MPI_Scatter and MPI_Gather to distribute the workload among processes
MPI_Scatter((rank == 0 ? grid : nullptr), chunk, MPI_DOUBLE, local, chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);

MPI_Gather(local, chunk, MPI_DOUBLE, (rank == 0 ? grid : nullptr), chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);
```

### Communication pattern

```cpp
// Exchange boundary data between processes, using asynchronuous communication
if (rank != 0)
    MPI_Irecv(prev, GRID_SIZE, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &req[req_count++]);
if (rank != size - 1)
    MPI_Irecv(next, GRID_SIZE, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &req[req_count++]);

if (rank != 0)
    MPI_Isend(local, GRID_SIZE, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &req[req_count++]);
if (rank != size - 1)
    MPI_Isend(&local[(GRID_SIZE / size - 1) * GRID_SIZE], GRID_SIZE, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &req[req_count++]);

MPI_Waitall(req_count, req, MPI_STATUSES_IGNORE);
```

### Synchronization

```cpp
// Use MPI_Reduce to accumulate the total uncontaminated cells
MPI_Reduce(&uncontaminated, &total_uncontaminated, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
```

## Performance

### Performance Metrics Calculation

```
Speedup = Sequential Time / Parallel Time
Efficiency = Speedup / Number of Processes
```

### Performance Comparision

| Implementation | Processes | Time (s) | Speedup | Efficiency |
| -------------- | --------- | -------- | ------- | ---------- |
| Sequential     | 1         | 24       | 1x      | 100%       |
| Parallel       | 2         | 10.6559  | 2.25x   | 113%       |
| Parallel       | 4         | 6.39454  | 3.75x   | 94%        |
| Parallel       | 8         | 7.62892  | 3.15x   | 40%        |

---

**Report Generated**: December 2024\
**Simulation Environment**: macOS ARM64, 8 cores, OpenMPI 5.0.8, GCC 15
