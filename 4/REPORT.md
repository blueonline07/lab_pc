# Distributed MPI Simulation Report

## Introduction

This project implements distributed MPI simulations for three physical phenomena
resulting from a DF61 ICBM detonation: heat diffusion, radioactive contamination
dispersion, and shock wave propagation. The simulations run across multiple
nodes using OpenMPI, comparing synchronous and asynchronous communication
patterns. The implementation uses a Docker-based multi-node cluster with 3
nodes, each running one MPI process.

## Architecture

### Distributed Setup

The simulation environment consists of:

- **3 Docker containers** (node1, node2, node3) running Ubuntu 22.04 with
  OpenMPI
- **Docker network** with fixed IP addresses (172.20.0.10, 172.20.0.11,
  172.20.0.12)
- **Passwordless SSH** configured between all nodes for MPI communication
- **Hostfile** (`hosts.txt`) listing all node IPs
- **Fixed process distribution**: 3 processes (1 per node) using
  `--map-by ppr:1:node`

## Implementation

The distributed MPI implementations use domain decomposition to partition the computational grid across multiple MPI processes running on different nodes. Each process handles a contiguous chunk of the grid, computing updates for its local portion. The key difference between synchronous and asynchronous implementations lies in the communication pattern used to exchange boundary data between neighboring processes.

### Domain Decomposition

All three simulations use a 1D row-wise decomposition strategy. The grid is divided into approximately equal chunks along the row dimension, with each process assigned a contiguous set of rows. Process 0 reads the input file and distributes the initial grid data using `MPI_Scatter`, and collects results using `MPI_Gather` at the end.

```cpp
int chunk = (N / size) * N;
MPI_Scatter((rank == 0 ? grid : nullptr), chunk, MPI_DOUBLE, 
            local, chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);
```

### Synchronous Communication Pattern

The synchronous implementation uses blocking MPI operations (`MPI_Sendrecv`) to exchange boundary data between neighboring processes. `MPI_Sendrecv` combines send and receive operations in a single call, ensuring that both operations complete before the function returns. This guarantees that boundary data is available before computation begins, but forces processes to wait for their neighbors, potentially causing idle time.

After each iteration, `MPI_Barrier` synchronizes all processes, ensuring that all computations complete before proceeding to the next time step. This strict synchronization maintains correctness but may limit performance when processes have varying workloads or network latencies.

```cpp
// Synchronous boundary exchange
if (rank != 0 && rank != size - 1) {
    MPI_Sendrecv(local, N, MPI_DOUBLE, rank - 1, 0,
                 prev, N, MPI_DOUBLE, rank - 1, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Sendrecv(&local[(N/size - 1) * N], N, MPI_DOUBLE, rank + 1, 0,
                 next, N, MPI_DOUBLE, rank + 1, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}
MPI_Barrier(MPI_COMM_WORLD);
```

### Asynchronous Communication Pattern

The asynchronous implementation uses non-blocking MPI operations (`MPI_Isend`, `MPI_Irecv`) to initiate communication and `MPI_Waitall` to wait for completion. This allows computation and communication to overlap, as processes can post receive requests, start sending data, and continue with computation while network transfers occur in the background.

The key advantage is that processes are not blocked waiting for neighbors—they can proceed with local computation while boundary data is being transferred. However, the implementation must ensure that boundary data is received before it is used in the computation, which is handled by `MPI_Waitall` before accessing the boundary arrays.

```cpp
// Asynchronous boundary exchange
MPI_Request req[4];
int req_count = 0;
if (rank != 0)
    MPI_Irecv(prev, N, MPI_DOUBLE, rank - 1, 0, 
              MPI_COMM_WORLD, &req[req_count++]);
if (rank != size - 1)
    MPI_Irecv(next, N, MPI_DOUBLE, rank + 1, 0, 
              MPI_COMM_WORLD, &req[req_count++]);
if (rank != 0)
    MPI_Isend(local, N, MPI_DOUBLE, rank - 1, 0, 
              MPI_COMM_WORLD, &req[req_count++]);
if (rank != size - 1)
    MPI_Isend(&local[(N/size - 1) * N], N, MPI_DOUBLE, rank + 1, 0, 
              MPI_COMM_WORLD, &req[req_count++]);
MPI_Waitall(req_count, req, MPI_STATUSES_IGNORE);
```

### Simulation 1: Heat Diffusion

The heat diffusion simulation uses a 3×3 convolution kernel to model heat spreading through a 2D grid. Each process computes the convolution for its local rows, requiring boundary data from neighboring processes for the top and bottom rows. The boundary conditions use a fixed temperature value (30.0) at the grid edges.

The computation involves accessing a 3×3 neighborhood around each cell, including diagonal neighbors. Boundary handling checks whether neighbors are within the local chunk or must be retrieved from adjacent processes.

### Simulation 2: Radioactive Dispersion

The radioactive dispersion simulation solves the advection-diffusion-decay equation using finite difference methods. Each process computes updates for its local rows, requiring only the immediate north and south neighbors (no diagonal dependencies). The simulation tracks uncontaminated cells and uses `MPI_Reduce` to aggregate the count across all processes.

The computation involves advection (wind-driven transport), diffusion (spreading), and decay/deposition terms. Boundary conditions use zero values at grid edges.

```cpp
double advection = WIND_X * (cur - n) / DX + WIND_Y * (cur - w) / DY;
double diffusion = DIFFUSION_COEFF * (s - 2*cur + n) / (DX*DX) + 
                   DIFFUSION_COEFF * (e - 2*cur + w) / (DY*DY);
double decay = DECAY_RATE * cur + DEPOSITION_RATE * cur;
cur = cur + TIME_STEP * (-advection + diffusion - decay);
MPI_Reduce(&uncontaminated, &total_uncontaminated, 1, 
           MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
```

### Simulation 3: Shock Wave Propagation

The shock wave simulation computes peak overpressure across the grid as the shock front expands from the detonation center. Unlike the other simulations, this computation is independent across time steps—each cell's value depends only on its distance from the center and the current time, not on neighboring cells. This eliminates the need for boundary exchange during computation.

The asynchronous version removes `MPI_Barrier` calls entirely, allowing processes to work independently. Data collection occurs only at the end using non-blocking send/receive operations. The synchronous version still uses barriers for consistency, though they are not strictly necessary for correctness.

```cpp
// Workload distribution with remainder handling
int rows_per_proc = N / size;
int remainder = N % size;
int start_row = rank * rows_per_proc + std::min(rank, remainder);
int end_row = start_row + rows_per_proc + (rank < remainder ? 1 : 0);
```

## Performance Analysis

### Performance Metrics Calculation

```
Speedup = Sequential Time / Parallel Time
Efficiency = Speedup / Number of Processes
```

### Performance Comparison

| Simulation     | Implementation | Hosts | Time (s) |
| -------------- | -------------- | ----- | -------- |
| Heat Diffusion | Sync           | 3     | 2.50     |
| Heat Diffusion | Async          | 3     | 2.47     |
| Radioactive    | Sync           | 3     | 2.77     |
| Radioactive    | Async          | 3     | 2.94     |
| Shock Wave     | Sync           | 3     | 34       |
| Shock Wave     | Async          | 3     | 33       |

_Note: Actual performance depends on Docker network characteristics, host system
resources, and network latency between containers._

---

**Report Generated**: December 2024\
**Simulation Environment**: Docker containers (Ubuntu 22.04), OpenMPI, 3-node
cluster, distributed execution\
**Process Distribution**: 3 processes (1 per node)
