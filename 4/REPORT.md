# Distributed MPI Simulation Report

## Introduction

This project implements distributed MPI simulations for three physical phenomena
resulting from a DF61 ICBM detonation: heat diffusion, radioactive contamination
dispersion, and shock wave propagation. The simulations run across multiple
nodes using OpenMPI, comparing synchronous and asynchronous communication
patterns. The implementation uses a Docker-based multi-node cluster with 3
nodes, each running one MPI process.

## Architecture

### Distributed Setup

The simulation environment consists of:

- **3 Docker containers** (node1, node2, node3) running Ubuntu 22.04 with
  OpenMPI
- **Docker network** with fixed IP addresses (172.20.0.10, 172.20.0.11,
  172.20.0.12)
- **Passwordless SSH** configured between all nodes for MPI communication
- **Hostfile** (`hosts.txt`) listing all node IPs
- **Fixed process distribution**: 3 processes (1 per node) using
  `--map-by ppr:1:node`

## Performance Analysis

### Performance Metrics Calculation

```
Speedup = Sequential Time / Parallel Time
Efficiency = Speedup / Number of Processes
```

### Performance Comparison

| Simulation     | Implementation | Hosts | Time (s) |
| -------------- | -------------- | ----- | -------- |
| Heat Diffusion | Sync           | 3     | 2.50     |
| Heat Diffusion | Async          | 3     | 2.47     |
| Radioactive    | Sync           | 3     | 2.77     |
| Radioactive    | Async          | 3     | 2.94     |
| Shock Wave     | Sync           | 3     | 34       |
| Shock Wave     | Async          | 3     | 33       |

_Note: Actual performance depends on Docker network characteristics, host system
resources, and network latency between containers._

---

**Report Generated**: December 2024\
**Simulation Environment**: Docker containers (Ubuntu 22.04), OpenMPI, 3-node
cluster, distributed execution\
**Process Distribution**: 3 processes (1 per node)
