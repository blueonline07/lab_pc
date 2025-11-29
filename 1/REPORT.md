# Heat Diffusion Simulation Report

## Introduction

This project implements a heat diffusion simulation using 2D convolution to
model the aftermath of a nuclear explosion. The simulation tracks how heat
spreads through a 4000×4000 matrix over 100 iterations, comparing sequential and
parallel implementations using OpenMP.

## Implementation Details

### Sequential Algorithm

```cpp
for (int t = 0; t < NUM_ITERS; t++)
{
    for (int i = 1; i <= GRID_SIZE; i++)
    {
        for (int j = 1; j <= GRID_SIZE; j++)
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

    double **temp = grid;
    grid = new_grid;
    new_grid = temp;
}
```

### Parallel Algorithm (OpenMP)

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

### Idea

- Eliminates boundary condition branches by working on a halo-padded domain
  `(N+2)×(N+2)` with fixed baseline temperature on the outer ring.
- Improves cache locality using 2D tiles (default 64×64), reducing cache misses
  and TLB pressure.
- Enables compiler vectorization with short, predictable inner loops.

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

| Implementation | Threads | Time (s) | Speedup vs Seq | Efficiency |
| -------------- | ------- | -------- | -------------- | ---------- |
| Tiled          | 4       | 13.5628  | 3.76x          | 94%        |

---

**Report Generated**: October 2025\
**Simulation Environment**: macOS 14, GCC 15.2, libomp 21.1, OpenMP, 8-core CPU
