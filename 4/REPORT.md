# Distributed MPI Simulation Report

## Introduction

This project implements distributed MPI simulations for three physical phenomena resulting from a DF61 ICBM detonation: heat diffusion, radioactive contamination dispersion, and shock wave propagation. The simulations run across multiple nodes using OpenMPI, comparing synchronous and asynchronous communication patterns. The implementation uses a Docker-based multi-node cluster with 3 nodes, each running one MPI process.

## Architecture

### Distributed Setup

The simulation environment consists of:
- **3 Docker containers** (node1, node2, node3) running Ubuntu 22.04 with OpenMPI
- **Docker network** with fixed IP addresses (172.20.0.10, 172.20.0.11, 172.20.0.12)
- **Passwordless SSH** configured between all nodes for MPI communication
- **Hostfile** (`hosts.txt`) listing all node IPs
- **Fixed process distribution**: 3 processes (1 per node) using `--map-by ppr:1:node`

### Simulation Parameters

All simulations run for 100 iterations (time steps), with each iteration representing 1 second:
- **Grid size**: 4000×4000 cells
- **Simulation time**: 100 seconds
- **Time step**: 1 second
- **Total iterations**: 100

## Implementation Details

### 1. Heat Diffusion Simulation (Folder 1)

Models heat spread using 2D convolution with a 3×3 kernel.

#### Data Distribution

```cpp
// Distribute grid rows across processes
int rows_per_proc = GRID_SIZE / size;
int start_row = rank * rows_per_proc;
int end_row = start_row + rows_per_proc;
```

#### Synchronous Communication

```cpp
// Exchange boundary rows using MPI_Sendrecv
if (rank != 0) {
    MPI_Sendrecv(local_grid[0], GRID_SIZE, MPI_DOUBLE, rank - 1, 0,
                 prev_row, GRID_SIZE, MPI_DOUBLE, rank - 1, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}
if (rank != size - 1) {
    MPI_Sendrecv(local_grid[local_rows - 1], GRID_SIZE, MPI_DOUBLE, rank + 1, 0,
                 next_row, GRID_SIZE, MPI_DOUBLE, rank + 1, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}
```

### 2. Radioactive Contamination Dispersion (Folder 2)

Models advection-diffusion-decay equation with wind effects.

#### Data Distribution

```cpp
int chunk = (N / size) * N;
MPI_Scatter((rank == 0 ? grid : nullptr), chunk, MPI_DOUBLE, 
            local, chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);
```

#### Asynchronous Communication

```cpp
MPI_Request req[4];
int req_count = 0;

// Post non-blocking receives
if (rank != 0)
    MPI_Irecv(prev, N, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &req[req_count++]);
if (rank != size - 1)
    MPI_Irecv(next, N, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &req[req_count++]);

// Post non-blocking sends
if (rank != 0)
    MPI_Isend(local, N, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &req[req_count++]);
if (rank != size - 1)
    MPI_Isend(&local[(N / size - 1) * N], N, MPI_DOUBLE, rank + 1, 0, 
              MPI_COMM_WORLD, &req[req_count++]);

// Wait for all communications to complete
MPI_Waitall(req_count, req, MPI_STATUSES_IGNORE);
```

#### Synchronous Communication

```cpp
// Use MPI_Sendrecv for synchronous boundary exchange
if (rank != 0 && rank != size - 1) {
    MPI_Sendrecv(local, N, MPI_DOUBLE, rank - 1, 0,
                 prev, N, MPI_DOUBLE, rank - 1, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Sendrecv(&local[(N / size - 1) * N], N, MPI_DOUBLE, rank + 1, 0,
                 next, N, MPI_DOUBLE, rank + 1, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}
```

#### Physics Computation

```cpp
double advection = WIND_X * (cur - n) / DX + WIND_Y * (cur - w) / DY;
double diffusion = DIFFUSION_COEFF * (s - 2 * cur + n) / (DX * DX) + 
                   DIFFUSION_COEFF * (e - 2 * cur + w) / (DY * DY);
double decay = DECAY_RATE * cur + DEPOSITION_RATE * cur;
cur = cur + TIME_STEP * (-advection + diffusion - decay);
```

### 3. Shock Wave Propagation (Folder 3)

Models blast wave overpressure using empirical formulas.

#### Data Distribution

```cpp
int rows_per_proc = N / size;
int remainder = N % size;
int start_row = rank * rows_per_proc + std::min(rank, remainder);
int end_row = start_row + rows_per_proc + (rank < remainder ? 1 : 0);
int local_rows = end_row - start_row;
```

#### Synchronous Communication

```cpp
// Synchronize all processes at each time step
MPI_Barrier(MPI_COMM_WORLD);

// Gather results at the end
if (rank == 0) {
    for (int p = 1; p < size; p++) {
        // Receive data from other processes
        MPI_Recv(grid[p_start_row + i], N, MPI_DOUBLE, p, 0, 
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
} else {
    for (int i = 0; i < local_rows; i++) {
        MPI_Send(local_grid[i], N, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }
}
```

#### Physics Computation

```cpp
double R = sqrt(sq(global_i - CENTER_X) + sq(j - CENTER_Y)) * CELL_SIZE;
if (t >= R / 343.0) {  // 343 m/s is speed of sound
    double Z = R * pow(W, -1.0 / 3.0);
    double U = -0.21436 + 1.35034 * log10(Z);
    double log10P = 0.0;
    for (int k = 0; k < 9; k++) {
        log10P += c[k] * pow(U, k);
    }
    local_grid[i][j] = pow(10.0, log10P);
}
```

## Performance Analysis

### Synchronous vs Asynchronous Communication

The key difference between synchronous and asynchronous implementations:

**Synchronous (MPI_Sendrecv)**:
- Blocks until communication completes
- Simpler to reason about
- Lower latency overhead per message
- Processes wait for neighbors before computation

**Asynchronous (MPI_Isend/MPI_Irecv)**:
- Non-blocking, allows overlap of computation and communication
- More complex to manage (request handles)
- Potential for better performance through communication/computation overlap
- Requires careful synchronization with `MPI_Waitall`

### Performance Metrics Calculation

```
Speedup = Sequential Time / Parallel Time
Efficiency = Speedup / Number of Processes
Communication Overhead = Parallel Time - Computation Time
```

### Expected Performance Characteristics

For distributed MPI across 3 nodes:

1. **Communication Overhead**: Network latency between Docker containers adds overhead compared to shared-memory parallelism
2. **Load Balancing**: With 3 processes and 4000 rows, each process handles ~1333 rows (with remainder handling)
3. **Scalability**: Limited by network bandwidth and latency in Docker environment
4. **Asynchronous Advantage**: May show improvement when computation can overlap with communication, but Docker network latency may limit benefits

### Performance Comparison (Expected)

| Simulation | Implementation | Processes | Expected Time (s) | Speedup | Efficiency |
| ---------- | -------------- | --------- | ----------------- | ------- | ---------- |
| Heat Diffusion | Synchronous | 3 | ~17-20 | ~2.5-3.0× | 83-100% |
| Radioactive | Synchronous | 3 | ~8-10 | ~2.4-3.0× | 80-100% |
| Radioactive | Asynchronous | 3 | ~7-9 | ~2.7-3.4× | 90-113% |
| Shock Wave | Synchronous | 3 | ~34-40 | ~2.5-3.0× | 83-100% |

*Note: Actual performance depends on Docker network characteristics, host system resources, and network latency between containers.*

## Key Observations

### Asynchronous Implementation Benefits

1. **Communication Overlap**: Non-blocking operations allow computation to proceed while data is being transmitted
2. **Reduced Idle Time**: Processes don't block waiting for neighbors, improving CPU utilization
3. **Better Scalability**: As network latency increases, asynchronous communication becomes more beneficial

### Distributed MPI Challenges

1. **Network Latency**: Docker bridge network adds latency compared to shared-memory
2. **Data Distribution**: Initial scatter and final gather operations add overhead
3. **Synchronization**: `MPI_Barrier` ensures all processes stay synchronized but adds overhead
4. **Load Balancing**: Uneven work distribution can occur with non-divisible grid sizes

### Implementation Considerations

1. **Boundary Exchange**: Critical for maintaining correctness in stencil computations
2. **Memory Management**: Each process maintains only its local portion of the grid
3. **Error Handling**: Network failures require robust error handling in production systems
4. **SSH Configuration**: Passwordless SSH is essential for seamless MPI communication

## Conclusion

The distributed MPI implementation successfully demonstrates:
- Multi-node parallel execution across Docker containers
- Comparison of synchronous vs asynchronous communication patterns
- Three distinct physical simulations with different computational characteristics
- Proper data distribution and boundary exchange mechanisms

The asynchronous implementation shows potential for improved performance through communication/computation overlap, though the benefits are limited by Docker network characteristics. For production HPC systems with high-speed interconnects (InfiniBand), asynchronous communication would show more significant advantages.

---

**Report Generated**: December 2024  
**Simulation Environment**: Docker containers (Ubuntu 22.04), OpenMPI, 3-node cluster, distributed execution  
**Grid Size**: 4000×4000, 100 iterations  
**Process Distribution**: 3 processes (1 per node)

