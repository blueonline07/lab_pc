# Radioactive Dispersion Simulation Report

## Introduction

This project implements a radioactive dispersion simulation using finite difference methods to solve the advection-diffusion-decay equation. The simulation models how radioactive contamination spreads through the atmosphere under wind conditions, accounting for advection, diffusion, decay, and deposition processes.

### Physical Model

The simulation solves the partial differential equation:

```
∂C/∂t + u·∇C = D∇²C - (λ + μ)C
```

Where:

- `C` = concentration of radioactive material
- `u` = wind velocity vector (ux, uy)
- `D` = diffusion coefficient
- `λ` = decay rate
- `μ` = deposition rate

### Simulation Parameters

| Parameter             | Value        | Description                  |
| --------------------- | ------------ | ---------------------------- |
| Grid Size             | 4000×4000    | Computational domain         |
| Cell Area             | 100.0        | Area per grid cell           |
| Simulation Time       | 100 seconds  | Total simulation duration    |
| Time Step             | 1.0 second   | Time increment per iteration |
| Wind Velocity         | (0.33, 0.14) | Wind components              |
| Diffusion Coefficient | 1.0          | Material diffusivity         |
| Decay Rate            | 3×10⁻⁵       | Radioactive decay            |
| Deposition Rate       | 1×10⁻⁴       | Ground deposition            |
| Initial Contamination | 1000.0       | Initial concentration        |

## Formula Implementation

### Finite Difference Discretization

The PDE is discretized using finite differences on a regular grid:

#### Advection Term (Upwind Scheme)

```cpp
advection_x = WIND_X * (C[i][j] - C[i-1][j]) / DX
advection_y = WIND_Y * (C[i][j] - C[i][j-1]) / DY
```

#### Diffusion Term (Central Difference)

```cpp
diffusion = DIFFUSION_COEFF * (
    (C[i+1][j] - 2*C[i][j] + C[i-1][j]) / (DX²) +
    (C[i][j+1] - 2*C[i][j] + C[i][j-1]) / (DY²)
)
```

#### Time Integration (Forward Euler)

```cpp
C_new[i][j] = C[i][j] + TIME_STEP * (-advection + diffusion - decay)
```

Where `decay = (DECAY_RATE + DEPOSITION_RATE) * C[i][j]`

### Boundary Conditions

Zero-padding boundary conditions are applied:

- All boundary cells are set to zero concentration
- This represents clean air at domain edges

## Parallelization Strategy

### Domain Decomposition

The computational grid is divided among MPI processes using 1D domain decomposition:

- Each process handles `GRID_SIZE / num_processes` rows
- Ghost cells are added for boundary data exchange
- Data is scattered from process 0 to all processes

### Communication Pattern

```cpp
// Exchange boundary data between processes
if (rank > 0) {
    MPI_Sendrecv(send_top, recv_top, rank-1);
}
if (rank < size-1) {
    MPI_Sendrecv(send_bottom, recv_bottom, rank+1);
}
```

### Synchronization

- `MPI_Barrier()` ensures all processes complete each time step
- `MPI_Reduce()` aggregates uncontaminated block counts
- `MPI_Gather()` collects final results for output

### Implementation Details

1. **Temporary Grid**: Uses temporary grid for computation to match sequential behavior
2. **Boundary Conditions**: Applied to actual grid boundaries, not ghost cells
3. **Loop Bounds**: Consistent with sequential implementation
4. **Initial Placement**: Contamination placed at the domain center `(2000, 2000)`

## Performance

### Performance Metrics Calculation
```
Speedup = Sequential Time / Parallel Time
Efficiency = Speedup / Number of Processes
```

### Performance Comparision

| Implementation | Processes | Time (s) | Speedup | Efficiency |
| -------------- | --------- | -------- | ------- | ---------- |
| Sequential     | 1         | 1.833    | 1.00x   | 100%       |
| Parallel       | 1         | 3.406    | 0.54x   | 54%        |
| Parallel       | 2         | 1.779    | 1.03x   | 51%        |
| Parallel       | 4         | 0.979    | 1.87x   | 47%        |
| Parallel       | 8         | 1.242    | 1.48x   | 18%        |
| Parallel       | 10        | 1.110    | 1.65x   | 17%        |
| Parallel       | 16        | 1.122    | 1.63x   | 10%        |
| Parallel       | 32        | 1.137    | 1.61x   | 5%         |


---

**Report Generated**: December 2024  
**Simulation Environment**: macOS ARM64, OpenMPI 5.0.8, GCC 15
