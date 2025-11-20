# Lab 4: Multi-Host MPI with Docker Compose

This lab demonstrates running MPI programs across multiple hosts using Docker Compose.

## Overview

The setup includes:
- A simple MPI program that displays which host each process runs on
- Docker Compose configuration for multiple MPI hosts
- `hosts.txt` file for MPI hostfile configuration

## Prerequisites

- Docker and Docker Compose installed
- OpenMPI (for local testing, optional)

## Quick Start

### Using Docker Compose (Recommended)

1. **Start the Docker containers:**
   ```bash
   make docker-up
   ```
   This will build the Docker images (with SSH server) and start both containers.

2. **Build and run the MPI program:**
   ```bash
   make docker-run
   ```
   This will automatically:
   - Set up SSH keys for passwordless access between containers
   - Build the MPI program
   - Run it across both hosts

   Or run with different number of processes:
   ```bash
   make docker-run-2  # 2 processes
   make docker-run-4  # 4 processes
   ```

3. **Stop the containers:**
   ```bash
   make docker-down
   ```

### Manual Docker Commands

```bash
# Start containers
docker compose up -d

# Build the program
docker compose exec mpi-host-1 make

# Run MPI program across hosts
docker compose exec mpi-host-1 mpirun --allow-run-as-root --hostfile hosts.txt -np 4 build/mpi_multi_host

# Stop containers
docker compose down
```

### Local Testing (without Docker)

If you have OpenMPI installed locally:

```bash
# Build
make

# Run on single host
make run

# Run on multiple hosts (requires SSH access to hosts in hosts.txt)
make run-multi
```

## Configuration

### hosts.txt

The `hosts.txt` file specifies which hosts to use and how many processes each can run:

```
mpi-host-1 slots=2
mpi-host-2 slots=2
```

This means:
- `mpi-host-1` can run 2 MPI processes
- `mpi-host-2` can run 2 MPI processes
- Total capacity: 4 processes

### docker-compose.yml

The Docker Compose file creates:
- Two containers: `mpi-host-1` and `mpi-host-2`
- A shared network: `mpi-network`
- Shared volume: current directory mounted to `/workspace`

## Expected Output

When running with 4 processes across 2 hosts, you should see:

```
=== MPI Multi-Host Execution ===
Total processes: 4
--------------------------------
Process 0 of 4 running on host: mpi-host-1 (MPI name: mpi-host-1)
Process 1 of 4 running on host: mpi-host-1 (MPI name: mpi-host-1)
Process 2 of 4 running on host: mpi-host-2 (MPI name: mpi-host-2)
Process 3 of 4 running on host: mpi-host-2 (MPI name: mpi-host-2)
--------------------------------
Execution completed successfully!
```

## Troubleshooting

### SSH Key Setup (for non-Docker multi-host)

If running on real multiple hosts (not Docker), you need SSH access:

```bash
# Generate SSH key if needed
ssh-keygen -t rsa

# Copy to remote hosts
ssh-copy-id user@host1
ssh-copy-id user@host2
```

### Docker Issues

- **Permission denied**: Make sure Docker is running and you have permissions
- **Network issues**: Check that containers can communicate: `docker network inspect lab_pc_mpi-network`
- **Build fails**: Ensure containers are running: `docker-compose ps`

## Make Targets

Run `make help` to see all available targets.

