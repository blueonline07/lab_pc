# Lab 4: Quick Start Guide

## Build

```bash
make
```

## Run

### Basic Execution (4 processes)

```bash
mpirun -np 4 ./build/mpi_shock_simulation
```

This will run both synchronous and asynchronous versions and compare performance.

### Run with 8 processes

```bash
mpirun -np 8 ./build/mpi_shock_simulation
```

### Skip CSV Output (Faster)

```bash
mpirun -np 4 ./build/mpi_shock_simulation --no-output
```

### Run Only Synchronous

```bash
mpirun -np 4 ./build/mpi_shock_simulation --sync-only
```

### Run Only Asynchronous

```bash
mpirun -np 4 ./build/mpi_shock_simulation --async-only
```

## Multi-Host Execution

1. Edit `hosts.txt`:
```
node1 slots=4
node2 slots=4
```

2. Run:
```bash
mpirun -np 8 --hostfile hosts.txt ./build/mpi_shock_simulation
```

## View Results

Results are saved as CSV files:
- `lab4_synchronous_result.csv` (120 MB)
- `lab4_asynchronous_result.csv` (120 MB)

## Expected Performance

| Processes | Synchronous | Asynchronous | Improvement |
|-----------|-------------|--------------|-------------|
| 4         | ~19 s       | ~18 s        | 5.5%        |
| 8         | ~16 s       | ~14 s        | 13.6%       |

Asynchronous version scales better with more processes!

## Help

```bash
./build/mpi_shock_simulation --help
```

## Full Documentation

- `README.md` - Complete usage guide
- `REPORT.md` - Detailed performance analysis
- `VALIDATION_SUMMARY.txt` - Test results

