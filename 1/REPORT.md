# Heat Diffusion Simulation Report

## Introduction

This project implements a heat diffusion simulation using 2D convolution to
model the aftermath of a nuclear explosion. The simulation tracks how heat
spreads through a 4000×4000 matrix over 100 iterations, comparing sequential and
parallel implementations using OpenMP.

## Implementation

The parallel implementations leverage OpenMP to distribute the computational workload across multiple threads. The core parallelization strategy focuses on the spatial domain decomposition of the 2D grid. Since each grid cell's update depends only on its 3×3 neighborhood from the previous iteration, the grid cells can be processed independently within each time step, making this an embarrassingly parallel problem at the per-iteration level.

The basic OpenMP implementation uses the `collapse(2)` clause to parallelize both nested loops iterating over the grid dimensions. This collapses the two-dimensional iteration space into a single loop, allowing OpenMP to distribute the combined iteration space more efficiently across available threads. Each thread processes a subset of grid cells, computing the convolution for its assigned cells and writing results to the new grid. The parallel region is synchronized at the end of each iteration before swapping the grid pointers, ensuring data consistency between time steps.

This approach provides good load balancing as each grid cell requires the same computational effort, and the static scheduling (default for `parallel for`) ensures roughly equal work distribution among threads. The parallelization overhead is minimal since the work per cell is substantial enough to amortize the cost of thread creation and synchronization.

### OpenMP implementation

```cpp
#pragma omp parallel for collapse(2)
for(int i = 1; i <= GRID_SIZE; i++) {
    for(int j = 1; j <= GRID_SIZE; j++) {
        // Same convolution logic as sequential
    }
}
```

## Performance

### Performance Metrics Calculation

```
Speedup    = T_seq / T_par
Efficiency = Speedup / Threads
```

### Performance Metrics

| Implementation | Threads | Time (s) | Speedup | Efficiency |
| -------------- | ------- | -------- | ------- | ---------- |
| Sequential     | 1       | 51       | 1.00×   | 100%       |
| OpenMP         | 4       | 13.8477  | 3.92×   | 98%        |
| OpenMP         | 8       | 9.57073  | 5.40×   | 67.5%      |

---

## Additional Parallel Approach: Tiled

The tiled approach extends the basic OpenMP parallelization with cache-aware optimization techniques. Instead of parallelizing individual grid cells, this strategy partitions the grid into fixed-size tiles (typically 64×64 cells) and parallelizes at the tile level. This two-level decomposition—tiles for parallelization and cells within tiles for sequential processing—provides several performance benefits.

First, tiling improves cache locality by ensuring that when a thread processes a tile, the data for that tile and its immediate neighbors (needed for the 3×3 convolution) fits within the CPU cache. This reduces cache misses and memory bandwidth requirements, which are often the bottleneck in stencil computations. The tile size is chosen to balance cache utilization with parallelization granularity—too small tiles increase overhead, while too large tiles may not fit in cache.

Second, the tiled approach works on a halo-padded domain `(N+2)×(N+2)` with fixed boundary conditions, eliminating conditional branches for boundary handling. This simplifies the inner loop structure and enables better compiler optimizations, including vectorization of the short, predictable inner loops.

Third, by using `schedule(static)` with tile-level parallelization, each thread processes a contiguous block of tiles, further improving spatial locality. The static scheduling ensures deterministic load distribution while maintaining good cache behavior as threads work on adjacent memory regions.

### Implementation

```cpp
for (int t = 0; t < NUM_ITERS; t++)
{

#pragma omp parallel for collapse(2) schedule(static)
    for (int ti = 1; ti <= GRID_SIZE; ti += TILE_SIZE)
    {
        for (int tj = 1; tj <= GRID_SIZE; tj += TILE_SIZE)
        {

            int i_end = std::min(ti + TILE_SIZE, GRID_SIZE + 1);
            int j_end = std::min(tj + TILE_SIZE, GRID_SIZE + 1);

            for (int i = ti; i < i_end; i++)
            {
                for (int j = tj; j < j_end; j++)
                {
                    double sum = 0;
                    for (int ki = 0; ki < 3; ki++)
                    {
                        for (int kj = 0; kj < 3; kj++)
                        {
                            int ni = i + ki - 1;
                            int nj = j + kj - 1;
                            sum += grid[ni][nj] * kernel[ki][kj];
                        }
                    }
                    new_grid[i][j] = sum;
                }
            }
        }
    }

    double **temp = grid;
    grid = new_grid;
    new_grid = temp;
}
```

### Results

Environment: 8 OpenMP threads

| Implementation | Threads | Time (s) | Speedup | Efficiency |
| -------------- | ------- | -------- | ------- | ---------- |
| Tiled          | 4       | 13.5628  | 3.76x   | 94%        |

---

**Report Generated**: December 2025\
**Simulation Environment**: macOS 14, GCC 15.2, libomp 21.1, OpenMP, 8-core CPU
