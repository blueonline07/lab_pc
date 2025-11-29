# Distributed MPI Docker Setup

This setup provides a simple Docker-based environment for running distributed MPI programs across multiple nodes.

## Prerequisites

- Docker
- Docker Compose

## Quick Start

1. **Build and start the containers:**
   ```bash
   docker-compose up -d
   ```

2. **Set up passwordless SSH between nodes:**
   ```bash
   chmod +x setup-ssh.sh
   ./setup-ssh.sh
   ```

3. **Run the test MPI program:**
   ```bash
   chmod +x run-mpi.sh
   ./run-mpi.sh 3
   ```

   This will run the MPI program with 3 processes (1 per node).

## Manual Execution

You can also run MPI commands manually:

```bash
# Run with 3 processes (1 per node)
docker exec mpi_node1 mpirun -np 3 --hostfile /app/hosts.txt --map-by ppr:1:node /app/test_mpi

# Run with 2 processes (will use 2 nodes)
docker exec mpi_node1 mpirun -np 2 --hostfile /app/hosts.txt --map-by ppr:1:node /app/test_mpi
```

## Architecture

- **3 Docker containers** (node1, node2, node3) running Ubuntu with OpenMPI
- **Docker network** with fixed IP addresses (10.1.11.1, 10.1.11.2, 10.1.11.3)
- **SSH configured** for passwordless access between nodes
- **Hostfile** (`hosts.txt`) listing all node IPs

## Files

- `Dockerfile`: Container image with OpenMPI and SSH
- `docker-compose.yml`: Multi-node cluster configuration
- `src/test_mpi.cpp`: Simple MPI test program
- `Makefile`: Build configuration
- `hosts.txt`: List of node IP addresses
- `setup-ssh.sh`: Script to configure passwordless SSH
- `run-mpi.sh`: Script to execute MPI programs

## Test Program

The `test_mpi.cpp` program:
- Prints hello messages from each MPI process
- Shows processor name, rank, and total processes
- Performs a simple send/receive communication test
- Uses MPI_Barrier for synchronization

## Cleanup

To stop and remove containers:
```bash
docker-compose down
```

To remove images:
```bash
docker-compose down --rmi all
```

