#!/bin/bash

# Script to rebuild MPI programs in all containers
# Usage: ./rebuild.sh [folder]
#   folder: 1, 2, or 3 (rebuilds all if not specified)

FOLDER=${1:-"all"}

echo "Rebuilding MPI programs..."

if [ "$FOLDER" == "1" ] || [ "$FOLDER" == "all" ]; then
    echo "Building folder 1 (sync)..."
    for node in mpi_node1 mpi_node2 mpi_node3; do
        echo "  Building on $node..."
        docker exec $node bash -c "cd /app && rm -f src/1/*.o sync_1 && make clean && make SRCDIR=src/1 TARGET=sync && mv sync sync_1"
    done
fi

if [ "$FOLDER" == "2" ] || [ "$FOLDER" == "all" ]; then
    echo "Building folder 2 (async and sync)..."
    for node in mpi_node1 mpi_node2 mpi_node3; do
        echo "  Building on $node..."
        docker exec $node bash -c "cd /app && rm -f src/2/*.o async_2 sync_2 && make clean && make SRCDIR=src/2 TARGET=async && mv async async_2 && make clean && make SRCDIR=src/2 TARGET=sync && mv sync sync_2"
    done
fi

if [ "$FOLDER" == "3" ] || [ "$FOLDER" == "all" ]; then
    echo "Building folder 3 (async and sync)..."
    for node in mpi_node1 mpi_node2 mpi_node3; do
        echo "  Building on $node..."
        docker exec $node bash -c "cd /app && rm -f src/3/*.o async_3 sync_3 && make clean && make SRCDIR=src/3 TARGET=async && mv async async_3 && make clean && make SRCDIR=src/3 TARGET=sync && mv sync sync_3"
    done
fi

echo "Rebuild complete!"

