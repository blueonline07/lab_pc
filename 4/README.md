# Distributed MPI Setup and Test

This is a simple setup for testing distributed MPI execution across multiple nodes.

## Prerequisites

1. **OpenMPI installed on all nodes**
   ```bash
   # On Ubuntu/Debian
   sudo apt-get install openmpi-bin openmpi-common libopenmpi-dev
   
   # On macOS (using Homebrew)
   brew install openmpi
   ```

2. **Passwordless SSH configured between nodes** (for multi-node setup)

## Quick Start

### 1. Single-Node Testing (Localhost)

For testing on a single machine:

```bash
# Compile the test program
make

# Run with 4 processes on localhost
make run-local

# Or manually:
mpirun -np 4 ./bin/test_mpi
```

### 2. Multi-Node Setup

#### Step 1: Configure Hosts

Edit `hosts.txt` and add the IP addresses or hostnames of your cluster nodes:

```
10.1.11.1
10.1.11.2
10.1.11.3
```

#### Step 2: Set Up Passwordless SSH

Run the setup script to help configure SSH:

```bash
chmod +x setup-ssh.sh
./setup-ssh.sh
```

Or manually:

```bash
# Generate SSH key (if you don't have one)
ssh-keygen

# Copy SSH key to each node
ssh-copy-id <USERNAME>@<IP_1>
ssh-copy-id <USERNAME>@<IP_2>
ssh-copy-id <USERNAME>@<IP_3>
```

Test SSH connection:
```bash
ssh <USERNAME>@<IP_1> 'echo Connection successful'
```

#### Step 3: Distribute Files

**Important**: The source code and executable must be available on all nodes at the same path.

```bash
# Option 1: Use scp to copy files to each node
for host in $(grep -v '^#' hosts.txt | grep -v '^$'); do
    scp -r . <USERNAME>@$host:/path/to/same/directory/
done

# Option 2: Use shared filesystem (NFS, etc.)
```

#### Step 4: Run Distributed MPI

```bash
# Compile (on each node or on shared filesystem)
make

# Run with one process per node
make run-distributed-one-per-node

# Or run with multiple processes (distributed across nodes)
make run-distributed

# Or manually specify number of processes:
mpirun -np 4 --hostfile hosts.txt ./bin/test_mpi
mpirun -np 3 --hostfile hosts.txt --map-by ppr:1:node ./bin/test_mpi
```

## Test Program

The `test_mpi.cpp` program:
- Prints process rank, total processes, and hostname for each process
- Performs a simple reduction operation to test MPI communication
- Verifies the computation is correct

## Makefile Targets

- `make` or `make all`: Compile the test program
- `make clean`: Remove compiled binaries
- `make run-local`: Run with 4 processes on localhost
- `make run-distributed`: Run distributed MPI (one process per node)
- `make run-distributed-one-per-node`: Run with `--map-by ppr:1:node` option

## Troubleshooting

1. **"Host key verification failed"**
   - Manually SSH to each host first to accept host keys
   - Or add hosts to `~/.ssh/known_hosts`

2. **"Command not found: mpirun"**
   - Ensure OpenMPI is installed and in PATH
   - Try: `which mpirun` or `which mpicc`

3. **"Permission denied (publickey)"**
   - SSH keys not properly distributed
   - Run `setup-ssh.sh` or manually copy keys

4. **"No route to host"**
   - Check network connectivity: `ping <IP>`
   - Verify firewall settings

5. **Executable not found on remote nodes**
   - Ensure executable is at the same path on all nodes
   - Use absolute paths in hostfile if needed

## Notes

- For single-node testing, you can use `localhost` in `hosts.txt`
- The `--map-by ppr:1:node` option ensures exactly one process per node
- Without this option, MPI may distribute processes unevenly across nodes

