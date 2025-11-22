# Lab 4: Docker Multi-Host MPI - Quick Start

## Prerequisites

- Docker Desktop or Docker Engine installed
- Docker Compose installed
- At least 4 GB RAM allocated to Docker

## One-Command Run

```bash
./run-docker.sh
```

This single command will:
1. Build Docker images (if needed)
2. Start 4 containers
3. Configure SSH between nodes
4. Build the simulation
5. Run with 12 processes across 4 containers

## Common Commands

### Basic Usage

```bash
# Run with default settings (12 processes)
./run-docker.sh

# Run with 4 processes (1 per container)
./run-docker.sh -np 4

# Run with 8 processes (2 per container)
./run-docker.sh -np 8

# Run without CSV output (faster)
./run-docker.sh --no-output
```

### Management

```bash
# Check cluster status
./run-docker.sh --status

# Open interactive shell
./run-docker.sh --shell

# Clean up everything
./run-docker.sh --clean
```

### Manual Control

```bash
# Start cluster
docker compose up -d

# Setup SSH
./setup-ssh.sh

# Build program
docker compose exec mpi-head make

# Run simulation
docker compose exec mpi-head \
  mpirun -np 12 --hostfile hosts-docker.txt \
  ./build/mpi_shock_simulation

# Stop cluster
docker compose down
```

## Performance

| Processes | Containers | Expected Time |
|-----------|------------|---------------|
| 4         | 4 (1 each) | ~60-90 s      |
| 8         | 4 (2 each) | ~30-40 s      |
| 12        | 4 (3 each) | ~20-25 s      |

**Async vs Sync:** Expect 30-40% improvement with asynchronous execution.

## Troubleshooting

### Containers not starting

```bash
# Force clean and rebuild
docker compose down
docker compose up -d --build
```

### SSH connection failed

```bash
# Re-run SSH setup
./setup-ssh.sh

# Or manually test
docker compose exec mpi-head ssh mpi-node-1 hostname
```

### Build errors

```bash
# Clean and rebuild
docker compose exec mpi-head bash -c "cd /workspace && make clean && make"
```

### Process distribution issues

```bash
# Check hostfile
docker compose exec mpi-head cat hosts-docker.txt

# Verify SSH to all nodes
docker compose exec mpi-head bash -c "
  for host in mpi-node-1 mpi-node-2 mpi-node-3; do
    ssh \$host hostname
  done
"
```

## Expected Output

```
========================================
  Multi-Host MPI Shock Wave Simulation
========================================
Total MPI processes: 12
Grid size: 4000 x 4000
Time steps: 100
Yield: 5000 kt
========================================

Process distribution:
  Process 0 running on: mpi-head
  Process 1 running on: mpi-head
  Process 2 running on: mpi-head
  Process 3 running on: mpi-node-1
  ...
  Process 11 running on: mpi-node-3

Running SYNCHRONOUS simulation...
Synchronous: Step 10/100 completed
...
Synchronous execution time: ~29 s

Running ASYNCHRONOUS simulation...
Asynchronous: Step 10/100 completed
...
Asynchronous execution time: ~20 s

Performance Comparison:
  Speedup:      1.47x
  Improvement:  32%
```

## Files

- `Dockerfile` - Container image definition
- `docker-compose.yml` - 4-node cluster configuration
- `hosts-docker.txt` - MPI hostfile
- `setup-ssh.sh` - SSH configuration script
- `run-docker.sh` - Convenience wrapper

## Documentation

- `README.md` - Complete project documentation
- `DOCKER_DEPLOYMENT.md` - Detailed deployment guide
- `DOCKER_VALIDATION.md` - Test results and validation
- `DOCKER_QUICK_START.md` - This file

## Tips

- First run takes longer (building images)
- Subsequent runs are much faster
- Use `--no-output` for benchmarking
- Increase process count for better async benefit
- Docker adds ~2-3% overhead vs native

## Scaling

To add more containers, edit `docker-compose.yml` and add:

```yaml
  mpi-node-4:
    build: {context: ., dockerfile: Dockerfile}
    hostname: mpi-node-4
    container_name: mpi-node-4
    volumes: [".:/workspace"]
    networks: ["mpi-network"]
    # ... (copy from existing nodes)
```

Update `setup-ssh.sh` and `hosts-docker.txt` accordingly.

## Next Steps

1. Experiment with different process counts
2. Add more containers (nodes)
3. Deploy to real HPC cluster when ready
4. Optimize for your specific use case

---

**Need Help?** See `DOCKER_DEPLOYMENT.md` for detailed instructions.

