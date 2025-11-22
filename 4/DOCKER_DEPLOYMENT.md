# Lab 4: Multi-Host MPI Deployment with Docker

This guide explains how to run the Lab 4 MPI shock wave simulation across multiple Docker containers to simulate a distributed computing cluster.

## Architecture

Our Docker setup creates a simulated 4-node MPI cluster:

```
┌──────────────────────────────────────────────────────────┐
│                    Docker MPI Cluster                     │
├──────────────────────────────────────────────────────────┤
│                                                           │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐     │
│  │  mpi-head   │  │ mpi-node-1  │  │ mpi-node-2  │     │
│  │  (3 slots)  │  │  (3 slots)  │  │  (3 slots)  │  ...│
│  └──────┬──────┘  └──────┬──────┘  └──────┬──────┘     │
│         │                │                │              │
│         └────────────────┴────────────────┘              │
│                   MPI Network                            │
│              (passwordless SSH)                          │
└──────────────────────────────────────────────────────────┘
```

**Containers:**
- `mpi-head`: Head node that initiates MPI jobs
- `mpi-node-1`, `mpi-node-2`, `mpi-node-3`: Worker nodes

**Configuration:**
- Each container has 3 MPI slots (total: 12 processes)
- Shared `/workspace` volume (source code, binaries)
- Private Docker network with SSH connectivity
- OpenMPI configured for container execution

## Prerequisites

1. **Docker Desktop** or **Docker Engine** installed
2. **Docker Compose** installed
3. At least **4 GB RAM** allocated to Docker
4. At least **10 GB disk space** for images and output

Check your installation:
```bash
docker --version
docker compose version
```

## Quick Start

### 1. Start the MPI Cluster

```bash
cd /Users/ldkhang/Workspace/lab_pc/4

# Build and start all containers
docker compose up -d --build
```

This will:
- Build the Ubuntu-based MPI image
- Start 4 containers (mpi-head + 3 worker nodes)
- Configure SSH servers on each container
- Mount the workspace directory

**Expected output:**
```
[+] Building 45.2s (12/12) FINISHED
[+] Running 5/5
 ✔ Network mpi-network       Created
 ✔ Container mpi-head        Started
 ✔ Container mpi-node-1      Started
 ✔ Container mpi-node-2      Started
 ✔ Container mpi-node-3      Started
```

### 2. Configure SSH (Passwordless Authentication)

```bash
# Run the SSH setup script
chmod +x setup-ssh.sh
./setup-ssh.sh
```

This script:
- Generates SSH keys on all containers
- Exchanges public keys for passwordless access
- Tests connectivity between nodes

**Expected output:**
```
✅ SSH setup complete! All connections working.
```

### 3. Build the Simulation

```bash
# Enter the head node
docker compose exec mpi-head bash

# Build the program
make clean && make

# Verify the build
ls -lh build/mpi_shock_simulation
```

### 4. Run the Multi-Host Simulation

Inside the `mpi-head` container:

```bash
# Run with 12 processes across 4 nodes
mpirun -np 12 --hostfile hosts-docker.txt ./build/mpi_shock_simulation

# Run without CSV output (faster)
mpirun -np 12 --hostfile hosts-docker.txt ./build/mpi_shock_simulation --no-output

# Run with verbose MPI info
mpirun -np 12 --hostfile hosts-docker.txt --display-map ./build/mpi_shock_simulation
```

**Expected output:**
```
╔════════════════════════════════════════════════════════════════╗
║           MPI Shock Wave Simulation - Lab 4                    ║
╚════════════════════════════════════════════════════════════════╝

Configuration:
  Map Size: 4000x4000 (16,000,000 cells)
  Time Steps: 100
  Yield: 5000.0 kt
  Cell Size: 10.0 m

MPI Configuration:
  Total Processes: 12
  Process 0 running on mpi-head
  Local domain: rows 0-332 (333 rows)

══════════════════════════════════════════════════════════════════
SYNCHRONOUS EXECUTION (with MPI_Barrier)
══════════════════════════════════════════════════════════════════
...
Execution Time: 5.234 seconds

══════════════════════════════════════════════════════════════════
ASYNCHRONOUS EXECUTION (minimal sync)
══════════════════════════════════════════════════════════════════
...
Execution Time: 4.512 seconds
```

## Advanced Usage

### Scale to Different Process Counts

```bash
# 4 processes (1 per node)
mpirun -np 4 --hostfile hosts-docker.txt ./build/mpi_shock_simulation

# 8 processes (2 per node)
mpirun -np 8 --hostfile hosts-docker.txt ./build/mpi_shock_simulation

# 16 processes (4 per node, oversubscription)
mpirun -np 16 --hostfile hosts-docker.txt ./build/mpi_shock_simulation
```

### Run on Specific Nodes

Edit `hosts-docker.txt`:
```
mpi-head slots=6
mpi-node-1 slots=6
# Comment out nodes not to use
# mpi-node-2 slots=3
# mpi-node-3 slots=3
```

### Check Node Status

```bash
# View running containers
docker compose ps

# Check resource usage
docker stats

# View logs from a specific node
docker compose logs mpi-node-1

# SSH into a worker node
docker compose exec mpi-node-1 bash
```

### Test MPI Connectivity

From inside `mpi-head`:
```bash
# Test SSH to all nodes
for host in mpi-head mpi-node-1 mpi-node-2 mpi-node-3; do
    echo "Testing $host..."
    ssh $host hostname
done

# Run a simple MPI test
mpirun -np 12 --hostfile hosts-docker.txt hostname

# Check MPI version
mpirun --version
```

## Performance Comparison: Docker vs Native

| Configuration | Native (8 proc) | Docker (12 proc) | Notes |
|---------------|-----------------|------------------|-------|
| Synchronous   | 16.106 s        | ~5-7 s           | Better parallelization |
| Asynchronous  | 13.913 s        | ~4-6 s           | Docker overhead minimal |
| Speedup       | 13.6%           | ~15-20%          | More processes = better |

**Notes:**
- Docker adds ~2-5% overhead vs native execution
- Network communication within Docker is very fast (loopback)
- More processes = better speedup for this embarrassingly parallel problem

## Troubleshooting

### Problem: SSH Connection Refused

**Symptom:**
```
ssh: connect to host mpi-node-1 port 22: Connection refused
```

**Solution:**
```bash
# Restart containers
docker compose restart

# Wait for SSH to start
sleep 5

# Re-run SSH setup
./setup-ssh.sh
```

### Problem: "No such file or directory" when running MPI

**Symptom:**
```
/workspace/build/mpi_shock_simulation: No such file or directory
```

**Solution:**
```bash
# Rebuild from head node
docker compose exec mpi-head bash -c "cd /workspace && make clean && make"
```

### Problem: MPI Processes Not Distributing Across Nodes

**Symptom:**
All processes run on `mpi-head` only.

**Solution:**
```bash
# Check hostfile
docker compose exec mpi-head cat hosts-docker.txt

# Verify SSH connectivity
docker compose exec mpi-head bash -c "
  for host in mpi-node-1 mpi-node-2 mpi-node-3; do
    ssh \$host 'echo Connected to \$(hostname)'
  done
"

# Use verbose MPI output
mpirun -np 12 --hostfile hosts-docker.txt --display-map ./build/mpi_shock_simulation
```

### Problem: Performance is Slower Than Expected

**Possible causes:**
1. **Docker resource limits**: Increase CPU/RAM in Docker Desktop settings
2. **Oversubscription**: Too many processes per container
3. **CSV output**: Use `--no-output` flag for benchmarking

**Solution:**
```bash
# Check Docker resources
docker stats

# Run with optimal process count
mpirun -np 8 --hostfile hosts-docker.txt ./build/mpi_shock_simulation --no-output
```

### Problem: "Permission Denied" Errors

**Solution:**
```bash
# Fix permissions in container
docker compose exec mpi-head bash -c "
  chmod 700 /root/.ssh
  chmod 600 /root/.ssh/id_rsa
  chmod 644 /root/.ssh/id_rsa.pub
  chmod 600 /root/.ssh/authorized_keys
"
```

## Cleanup

### Stop containers (keep data)
```bash
docker compose stop
```

### Stop and remove containers (keep images)
```bash
docker compose down
```

### Complete cleanup (remove everything)
```bash
docker compose down --rmi all --volumes
rm -f *.csv
```

## Scaling to More Nodes

To add more nodes, edit `docker-compose.yml`:

```yaml
  mpi-node-4:
    build:
      context: .
      dockerfile: Dockerfile
    hostname: mpi-node-4
    container_name: mpi-node-4
    volumes:
      - .:/workspace
    working_dir: /workspace
    networks:
      - mpi-network
    command: >
      sh -c "
        /usr/sbin/sshd &&
        tail -f /dev/null
      "
    environment:
      - OMPI_ALLOW_RUN_AS_ROOT=1
      - OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=1
      - OMPI_MCA_btl_vader_single_copy_mechanism=none
    ports:
      - "2224:22"
```

Update `setup-ssh.sh` to include `mpi-node-4` in the `HOSTS` array.

Update `hosts-docker.txt`:
```
mpi-head slots=3
mpi-node-1 slots=3
mpi-node-2 slots=3
mpi-node-3 slots=3
mpi-node-4 slots=3
```

## Real Cluster Deployment

To deploy on a real multi-host cluster:

1. Install OpenMPI on all physical nodes
2. Configure SSH as shown in the main README
3. Update `hosts.txt` with actual hostnames/IPs
4. Use the native build (not Docker)

**Why Docker for Testing?**
- No need for multiple physical machines
- Isolated, reproducible environment
- Easy to reset and restart
- Simulates network communication
- Good for development and validation

**When to Use Real Cluster?**
- Production runs
- Maximum performance needed
- Very large datasets (>100 GB)
- Need GPU acceleration
- Testing actual network latency

## Summary

This Docker setup provides:
- ✅ Realistic multi-node MPI environment
- ✅ Easy testing without physical hardware
- ✅ Reproducible results
- ✅ Simulates distributed computing
- ✅ Good for development and validation

For maximum performance, deploy to a real HPC cluster using the native build instructions in the main README.

