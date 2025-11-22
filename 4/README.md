# Lab 4: Multi-Host MPI Shock Wave Simulation

## Overview

This lab implements a distributed nuclear blast shock wave simulation using MPI across multiple machines. The simulation computes peak overpressure across a 4000×4000 grid as the shock front expands from a 5000 kt detonation center over 100 time steps.

Two execution methods are compared:
- **Synchronous**: Uses `MPI_Barrier` after each time step (high synchronization overhead)
- **Asynchronous**: Minimal synchronization, overlapping computation (better performance)

## Physics Model

Uses Lab 3's Kingery-Bulmash empirical relationships:
- Scaled distance: `Z = R · W^(-1/3)`
- Peak overpressure: `log10(Pso) = Σ c_i · u^i`
- Arrival time: `t = R / 343` (speed of sound)

## Requirements

- OpenMPI (or compatible MPI implementation)
- C++17 compiler
- SSH access between nodes (for multi-host execution)

## Build

```bash
make
```

## Usage

### Local Execution (Single Machine)

```bash
# Run with 4 processes
make run

# Run with 8 processes
mpirun -np 8 ./build/mpi_shock_simulation
```

### Multi-Host Execution

1. **Configure hosts file** (`hosts.txt`):
```
node1 slots=4
node2 slots=4
node3 slots=4
```

2. **Run with hostfile**:
```bash
mpirun -np 12 --hostfile hosts.txt ./build/mpi_shock_simulation
```

### Options

```bash
# Run only synchronous version
./build/mpi_shock_simulation --sync-only

# Run only asynchronous version
./build/mpi_shock_simulation --async-only

# Skip CSV output (faster)
./build/mpi_shock_simulation --no-output

# Show help
./build/mpi_shock_simulation --help
```

## Multi-Host Setup

### SSH Configuration

For password-less execution across nodes:

```bash
# On each node, generate SSH key
ssh-keygen

# Distribute public key to all other nodes
ssh-copy-id user@node1
ssh-copy-id user@node2
ssh-copy-id user@node3
```

### MPI Environment

Ensure OpenMPI is installed on all nodes with consistent versions:

```bash
# Check MPI installation
mpirun --version
which mpirun
```

## Output Files

- `lab4_synchronous_result.csv` - Results from synchronous execution
- `lab4_asynchronous_result.csv` - Results from asynchronous execution

Each file contains a 4000×4000 matrix of peak overpressure values (kPa).

## Performance Analysis

The asynchronous version should show better performance due to:
1. **Reduced synchronization overhead**: No barrier after each time step
2. **Better CPU utilization**: Processes work independently
3. **Lower communication cost**: Minimal synchronization points

Expected speedup: 5-20% improvement over synchronous version.

## Simulation Parameters

| Parameter | Value |
|-----------|-------|
| Grid Size | 4000×4000 |
| Cell Size | 10.0 m |
| Time Steps | 100 |
| Yield | 5000 kt |
| Physics Model | Kingery-Bulmash |

## Docker Multi-Node Deployment

For testing multi-node MPI without physical hardware, use Docker containers:

### Quick Start with Docker

```bash
# Method 1: Using the convenience script (recommended)
./run-docker.sh

# Method 2: Manual setup
docker compose up -d --build    # Start 4 containers
./setup-ssh.sh                  # Configure SSH
docker compose exec mpi-head bash
make && mpirun -np 12 --hostfile hosts-docker.txt ./build/mpi_shock_simulation
```

### Docker Features

- ✅ **4 containers** simulating a distributed cluster (`mpi-head` + 3 worker nodes)
- ✅ **12 processes** total (3 per container)
- ✅ **Automatic SSH setup** between nodes
- ✅ **Shared workspace** mounted in all containers
- ✅ **Easy scaling** - add more nodes in `docker-compose.yml`

### Docker Commands

```bash
# Run with custom process count
./run-docker.sh -np 8

# Run without CSV output
./run-docker.sh --no-output

# Open shell in head node
./run-docker.sh --shell

# Check cluster status
./run-docker.sh --status

# Clean up containers
./run-docker.sh --clean
```

**📘 For detailed Docker deployment instructions, see [DOCKER_DEPLOYMENT.md](DOCKER_DEPLOYMENT.md)**

## Troubleshooting

### "Could not resolve hostname"
- Check `hosts.txt` has correct hostnames
- Verify SSH connectivity: `ssh hostname`

### "Exited with exit code 255"
- SSH authentication failed
- Ensure password-less SSH is configured

### Different results between runs
- Results should be identical (deterministic)
- If different, check MPI version consistency

## References

- Lab 3 physics implementation
- OpenMPI documentation: https://www.open-mpi.org/
- Kingery-Bulmash blast wave parameters
