#!/bin/bash

# Script to run MPI program across distributed nodes
# Usage: ./run-mpi.sh [folder] [program_name] [input_file]
#   folder: 1, 2, or 3 (which source folder to use, default: 2)
#   program_name: sync or async (async available in folders 2 and 3, default: sync)
#   input_file: Input CSV file (required for folders 1 and 2, ignored for folder 3)
# Note: Always uses 3 processes (1 per node)
# Note: Folder 3 does not require an input file

NUM_PROCESSES=3
FOLDER=${1:-2}
PROGRAM=${2:-sync}
INPUT_FILE=${3:-input.csv}

# Validate folder
if [ "$FOLDER" != "1" ] && [ "$FOLDER" != "2" ] && [ "$FOLDER" != "3" ]; then
    echo "Error: Folder must be 1, 2, or 3"
    exit 1
fi

# Validate program for folder 1 (only sync available)
if [ "$FOLDER" == "1" ] && [ "$PROGRAM" == "async" ]; then
    echo "Error: async is not available in folder 1, only sync"
    exit 1
fi

# Determine executable name
if [ "$FOLDER" == "1" ]; then
    EXECUTABLE="sync_1"
elif [ "$FOLDER" == "3" ]; then
    if [ "$PROGRAM" == "async" ]; then
        EXECUTABLE="async_3"
    else
        EXECUTABLE="sync_3"
    fi
else
    if [ "$PROGRAM" == "async" ]; then
        EXECUTABLE="async_2"
    else
        EXECUTABLE="sync_2"
    fi
fi

echo "Running MPI program from folder $FOLDER: '$PROGRAM' with $NUM_PROCESSES processes across distributed nodes..."
echo "Executable: $EXECUTABLE"

# Copy input file to containers if needed (folders 1 and 2 only)
CONTAINER_INPUT_FILE="/app/input.csv"
if [ "$FOLDER" != "3" ]; then
    echo "Using input file: $INPUT_FILE"
    if [ -f "$INPUT_FILE" ]; then
        echo "Copying input file to containers..."
        docker cp "$INPUT_FILE" mpi_node1:$CONTAINER_INPUT_FILE
        docker cp "$INPUT_FILE" mpi_node2:$CONTAINER_INPUT_FILE
        docker cp "$INPUT_FILE" mpi_node3:$CONTAINER_INPUT_FILE
    else
        echo "Warning: Input file '$INPUT_FILE' not found. Make sure it exists in the containers."
    fi
    # Run MPI program with input file
    docker exec mpi_node1 bash -c "cd /app && mpirun --allow-run-as-root -np $NUM_PROCESSES --hostfile hosts.txt --map-by ppr:1:node ./$EXECUTABLE $CONTAINER_INPUT_FILE"
else
    echo "Note: Folder 3 does not require an input file"
    # Run MPI program without input file
    docker exec mpi_node1 bash -c "cd /app && mpirun --allow-run-as-root -np $NUM_PROCESSES --hostfile hosts.txt --map-by ppr:1:node ./$EXECUTABLE"
fi

