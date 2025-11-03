# Heat Diffusion Simulation Report
## Introduction

This project implements a heat diffusion simulation using 2D convolution to model the aftermath of a nuclear explosion. The simulation tracks how heat spreads through a 4000×4000 matrix over 100 iterations, comparing sequential and parallel implementations using OpenMP.

### Physical Model

The simulation models heat diffusion using the convolution operation:

```
T_new[i,j] = Σ(kernel[ki,kj] × T_old[i+ki-1, j+kj-1])
```

Where:

- `T` = temperature distribution
- `kernel` = 3×3 heat diffusion kernel
- Heat spreads from high-temperature regions to cooler areas
- Boundary conditions maintain baseline temperature of 30°C

### Simulation Parameters

| Parameter             | Value     | Description                   |
| --------------------- | --------- | ----------------------------- |
| Grid Size             | 4000×4000 | Computational domain          |
| Cell Area             | 100 m²    | Area per grid cell            |
| Simulation Iterations | 100       | Heat diffusion steps          |
| Kernel Size           | 3×3       | Convolution kernel            |
| Baseline Temperature  | 30°C      | Ambient temperature           |
| Peak Temperature      | 1000°C    | Initial epicenter temperature |

### Heat Diffusion Kernel

The 3×3 kernel used for heat diffusion:

```cpp
const float HEAT_KERNEL[3][3] = {
    {0.05f, 0.1f, 0.05f},
    {0.1f,  0.4f, 0.1f},
    {0.05f, 0.1f, 0.05f}
};
```

This kernel ensures:

- Heat conservation (sum = 1.0)
- Smooth diffusion in all directions
- Stronger influence from center cell (0.4) than neighbors (0.1, 0.05)

## Implementation Details

### Sequential Algorithm

```cpp
for(int iter = 0; iter < ITERATIONS; iter++) {
    memset(output, 0, N * N * sizeof(float));

    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            float sum = 0.0f;
            for(int ki = 0; ki < 3; ki++) {
                for(int kj = 0; kj < 3; kj++) {
                    int in_i = i + ki - 1;
                    int in_j = j + kj - 1;

                    if(in_i >= 0 && in_i < N && in_j >= 0 && in_j < N) {
                        sum += input[in_i * N + in_j] * kernel[ki][kj];
                    } else {
                        sum += BASELINE_TEMP * kernel[ki][kj];
                    }
                }
            }
            output[i * N + j] = sum;
        }
    }
    swap(input, output);
}
```

### Parallel Algorithm (OpenMP)

```cpp
#pragma omp parallel for collapse(2) schedule(static)
for(int i = 0; i < N; i++) {
    for(int j = 0; j < N; j++) {
        // Same convolution logic as sequential
    }
}
```

**Parallelization Strategy:**

- `collapse(2)`: Parallelizes both outer loops simultaneously
- `schedule(static)`: Distributes work evenly among threads
- No data dependencies between output pixels
- Each thread computes independent regions

## Parallelization Strategy

### Work Distribution

- OpenMP parallel-for over the two spatial loops with `collapse(2)`
- Static scheduling to evenly divide the iteration space
- No data dependencies between output cells (read-only input, write-only output)

### Synchronization

- Implicit barrier at the end of the parallel for
- Ping-pong buffers avoid race conditions between iterations

### Implementation Details

1. Temporary output buffer each iteration, then swap with input
2. Boundary handling applied per-access using baseline temperature
3. Warm cache-friendly access pattern (row-major, contiguous writes)

## Performance

### Performance Metrics Calculation

```
Speedup    = T_seq / T_par
Efficiency = Speedup / Threads
```

### Performance Metrics

| Implementation | Threads | Time (s) | Speedup | Efficiency |
| -------------- | ------- | -------- | ------- | ---------- |
| Sequential     | 1       | 2.834    | 1.00×   | 100%       |
| Parallel       | 4       | 1.279    | 2.21×   | 55.4%      |
| Parallel       | 8       | 1.259    | 2.24×   | 28.1%      |

---

## Additional Parallel Approach: Tiled + Halo Padding + SIMD

### Rationale

- Eliminates boundary condition branches by working on a halo-padded domain `(N+2)×(N+2)` with fixed baseline temperature on the outer ring.
- Improves cache locality using 2D tiles (default 64×64), reducing cache misses and TLB pressure.
- Enables compiler vectorization with short, predictable inner loops.

### Core Idea (pseudocode)

```cpp
// stride = N + 2, tile = 64, cur_in/out are halo-padded
for (int iter = 0; iter < I; ++iter) {
  set_halo(cur_out, BASELINE_TEMP);
  #pragma omp parallel for collapse(2) schedule(static)
  for (int i0 = 0; i0 < N; i0 += tile)
    for (int j0 = 0; j0 < N; j0 += tile)
      for (int i = i0; i < min(i0+tile, N); ++i)
        for (int j = j0; j < min(j0+tile, N); ++j) {
          int ii = i + 1, jj = j + 1; // halo offset
          float s = 0.f;
          s += cur_in[(ii-1)*S + (jj-1)] * K[0][0];
          s += cur_in[(ii-1)*S + (jj  )] * K[0][1];
          s += cur_in[(ii-1)*S + (jj+1)] * K[0][2];
          s += cur_in[(ii  )*S + (jj-1)] * K[1][0];
          s += cur_in[(ii  )*S + (jj  )] * K[1][1];
          s += cur_in[(ii  )*S + (jj+1)] * K[1][2];
          s += cur_in[(ii+1)*S + (jj-1)] * K[2][0];
          s += cur_in[(ii+1)*S + (jj  )] * K[2][1];
          s += cur_in[(ii+1)*S + (jj+1)] * K[2][2];
          cur_out[ii*S + jj] = s;
        }
  swap(cur_in, cur_out);
}
```
### Results

Environment: 8 OpenMP threads

| Implementation       | Threads | Time (s) | Speedup vs Seq | Efficiency |
| -------------------- | ------- | -------- | -------------- | ---------- |
| Sequential           | 1       | 2.839    | 1.00×          | 100%       |
| Parallel collapse(2) | 8       | 1.156    | 2.45×          | 30.7%      |
| Tiled + Halo         | 8       | 0.457    | 6.22×          | 77.7%      |

---
**Report Generated**: October 2025  
**Simulation Environment**: macOS 14, GCC 15.2, libomp 21.1, OpenMP, 8-core CPU
