#!/bin/bash

# Script to run MPI program across distributed nodes

NUM_PROCESSES=${1:-3}

echo "Running MPI program with $NUM_PROCESSES processes across distributed nodes..."

# Run MPI program on node1 (master node)
docker exec mpi_node1 bash -c "cd /app && mpirun --allow-run-as-root -np $NUM_PROCESSES --hostfile hosts.txt --map-by ppr:1:node ./test_mpi"

