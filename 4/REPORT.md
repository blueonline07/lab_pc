# Multi-Host MPI Shock Wave Simulation Report

## Introduction

This project implements a distributed nuclear blast shock wave simulation using MPI across multiple machines. The simulation compares **synchronous** and **asynchronous** execution methods to analyze the performance impact of synchronization overhead in distributed computing.

### Simulation Scenario

The simulation models the shock wave propagation from a 5000 kiloton nuclear detonation across a 4000×4000 grid, computing peak overpressure at each point over 100 time steps.

## Physical Model

The simulation uses the Kingery-Bulmash empirical relationships for blast wave parameters (same as Lab 3):

### Scaled Distance
```
Z = R · W^(-1/3)
```

Where:
- `R` = distance from blast center (meters)
- `W` = yield in kg TNT equivalent
- `Z` = scaled distance

### Peak Overpressure
```
log10(Pso) = Σ(i=0 to 8) c_i · u^i
u = -0.21436 + 1.35034 · log10(Z)
```

Where `Pso` is peak overpressure in kPa.

### Arrival Time
```
t_arrival = R / 343
```

Simplified model assuming shock wave travels at speed of sound (343 m/s).

## Simulation Parameters

| Parameter | Value |
|-----------|-------|
| Grid Size | 4000×4000 |
| Cell Size | 10.0 m |
| Simulation Time | 100 time steps (seconds) |
| Yield | 5000 kt |
| MPI Processes | 4 (single machine test) |

## Implementation

### Synchronous Execution

The synchronous implementation uses `MPI_Barrier()` after each time step to ensure all processes complete before proceeding:

```cpp
for (int time_step = 0; time_step < TIME_STEPS; time_step++) {
    // Compute local region
    computeLocalRegion(time_step);
    
    // Synchronize all processes after each time step
    MPI_Barrier(MPI_COMM_WORLD);
}
```

**Characteristics:**
- **High synchronization overhead**: 100 barriers (one per time step)
- **Guaranteed consistency**: All processes advance together
- **Load imbalance impact**: Fast processes wait for slow ones
- **Predictable execution**: Deterministic timing

### Asynchronous Execution

The asynchronous implementation minimizes synchronization, allowing processes to work independently:

```cpp
for (int time_step = 0; time_step < TIME_STEPS; time_step++) {
    // Compute local region (no barrier)
    computeLocalRegion(time_step);
    
    // Continue immediately to next iteration
}

// Final gather (only synchronization point)
MPI_Gatherv(local_map, ...);
```

**Characteristics:**
- **Minimal synchronization**: Only at final gather
- **Better CPU utilization**: Processes work independently
- **Reduced waiting time**: No idle periods between iterations
- **Same correctness**: Results are identical (no inter-process dependencies)

### Domain Decomposition

The 4000×4000 grid is divided row-wise among processes:

```
Process 0: Rows 0-999
Process 1: Rows 1000-1999
Process 2: Rows 2000-2999
Process 3: Rows 3000-3999
```

Each process computes its assigned region independently. Since shock wave calculation is **embarrassingly parallel** (no inter-cell dependencies), processes don't need to communicate during computation.

## Performance Results

### Execution Time (4 Processes, Single Machine)

| Implementation | Time (s) | Speedup | Improvement |
|---------------|----------|---------|-------------|
| Synchronous   | 19.387   | 1.00×   | -           |
| Asynchronous  | 18.311   | 1.059× | 5.55%       |

### Analysis

1. **Synchronization Overhead**
   - Synchronous version includes 100 barriers
   - Each barrier adds ~10.76 ms overhead on average
   - Total overhead: ~1.076 seconds

2. **Performance Improvement**
   - Asynchronous version eliminates barrier overhead
   - 5.55% speedup demonstrates benefit of reducing synchronization
   - Improvement is modest because computation dominates communication

3. **Scalability Considerations**
   - On multi-host setup, improvement would be more significant
   - Network latency amplifies barrier overhead
   - Expected 10-20% improvement on distributed cluster

## Correctness Verification

### Output Validation

Both implementations produce **identical** results:

```
✓ Synchronous vs Asynchronous: IDENTICAL
```

### Physical Validation

```
Min overpressure:    0.19 kPa (far edge)
Max overpressure:    54,577.50 kPa (near center)
Corner overpressure: 5.74 kPa
```

- ✓ Values match Lab 3 (same physics model)
- ✓ Radial decay pattern from blast center
- ✓ No negative pressures
- ✓ Physically reasonable range

## Multi-Host Execution

### Setup Requirements

For true distributed execution across multiple machines:

1. **SSH Configuration**: Password-less SSH between all nodes
2. **MPI Installation**: Consistent OpenMPI version on all hosts
3. **Shared Filesystem** (optional): NFS or similar for easy file access
4. **Hostfile Configuration**: List of machines with slot counts

### Example Hostfile

```
node1 slots=4
node2 slots=4
node3 slots=4
```

### Execution Command

```bash
mpirun -np 12 --hostfile hosts.txt \
       --map-by ppr:1:node \
       ./build/mpi_shock_simulation
```

The `--map-by ppr:1:node` option ensures exactly 1 process per node for better load balancing across machines.

## Performance Comparison with Lab 3

| Metric | Lab 3 (Pthreads) | Lab 4 (MPI) |
|--------|------------------|-------------|
| Implementation | Work-pool model | Domain decomposition |
| Parallelism | Single machine | Multi-machine capable |
| Overhead | Thread creation | Process communication |
| Time (4 workers/processes) | ~7.75 s | ~18.31 s (async) |

**Note**: Lab 4 is slower on single machine because:
- MPI process creation overhead vs threads
- Data gathering overhead (MPI_Gatherv)
- No shared memory (must copy data)
- Lab 4 is designed for distributed execution, not optimized for single-machine

On a true multi-host cluster, Lab 4 would scale beyond single machine limits.

## Synchronous vs Asynchronous Trade-offs

### Synchronous Advantages
- ✓ Easier to debug (predictable execution order)
- ✓ Simpler to reason about (all processes in sync)
- ✓ Better for algorithms with dependencies

### Synchronous Disadvantages
- ✗ Higher overhead (frequent barriers)
- ✗ Load imbalance amplified (fast processes wait)
- ✗ Poor scaling on high-latency networks

### Asynchronous Advantages
- ✓ Lower overhead (minimal synchronization)
- ✓ Better CPU utilization
- ✓ Scales better on distributed systems
- ✓ Tolerates load imbalance better

### Asynchronous Disadvantages
- ✗ Harder to debug (non-deterministic order)
- ✗ Requires careful design (avoid race conditions)
- ✗ Not suitable for tightly coupled algorithms

## Conclusions

1. **Asynchronous execution provides measurable performance improvement** (5.55% on single machine, potentially 10-20% on cluster)

2. **Both methods produce identical results**, demonstrating correctness

3. **For embarrassingly parallel problems**, asynchronous execution is clearly superior due to minimal communication requirements

4. **MPI enables true distributed computing** across multiple machines, allowing simulations to scale beyond single-machine memory and compute limits

5. **Synchronization overhead is significant** even on single machine (1.076 seconds out of 19.387 seconds = 5.55%)

## Recommendations

- **Use asynchronous execution** for production runs on distributed clusters
- **Use synchronous execution** for debugging or when debugging distributed issues
- **Scale to more nodes** to handle larger grids or finer resolution
- **Consider hybrid MPI+OpenMP** for optimal performance on multi-core clusters

---

**Report Generated**: November 2024  
**Simulation Environment**: macOS, OpenMPI 5.0.8, C++17  
**Test Configuration**: 4 processes, single machine  
**Grid Size**: 4000×4000 (16 million cells)  
**Yield**: 5000 kt nuclear detonation

