#include "shock_simulation.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>

ShockSimulation::ShockSimulation(int rank, int size) 
    : rank(rank), size(size) {
    
    // Calculate local domain for this process
    local_rows = MAP_SIZE / size;
    start_row = rank * local_rows;
    end_row = (rank == size - 1) ? MAP_SIZE : start_row + local_rows;
    local_rows = end_row - start_row;
    
    // Allocate local map
    local_map = new double[local_rows * MAP_SIZE];
    std::fill(local_map, local_map + local_rows * MAP_SIZE, 0.0);
    
    // Allocate global map only on rank 0
    if (rank == 0) {
        global_map = new double[MAP_SIZE * MAP_SIZE];
        std::fill(global_map, global_map + MAP_SIZE * MAP_SIZE, 0.0);
    } else {
        global_map = nullptr;
    }
}

ShockSimulation::~ShockSimulation() {
    delete[] local_map;
    if (global_map) {
        delete[] global_map;
    }
}

void ShockSimulation::computeLocalRegion(int time_step) {
    // Each process computes its assigned rows
    for (int local_i = 0; local_i < local_rows; local_i++) {
        int global_row = start_row + local_i;
        for (int col = 0; col < MAP_SIZE; col++) {
            // Calculate distance from blast center
            double distance = PhysicsUtils::calculateDistance(
                global_row, col, MAP_SIZE, CELL_SIZE);
            
            // Check if shock has reached this cell
            if (PhysicsUtils::hasShockReached(distance, time_step)) {
                // Calculate and store peak overpressure
                double overpressure = PhysicsUtils::calculatePeakOverpressure(
                    distance, YIELD);
                local_map[local_i * MAP_SIZE + col] = overpressure;
            }
        }
    }
}

double ShockSimulation::runSynchronous() {
    double start_time = MPI_Wtime();
    
    // Synchronous execution: barrier after each time step
    for (int time_step = 0; time_step < TIME_STEPS; time_step++) {
        // Compute local region
        computeLocalRegion(time_step);
        
        // Synchronize all processes after each time step
        MPI_Barrier(MPI_COMM_WORLD);
        
        // Progress report from rank 0
        if (rank == 0 && (time_step + 1) % 10 == 0) {
            std::cout << "Synchronous: Step " << (time_step + 1) 
                      << "/" << TIME_STEPS << " completed" << std::endl;
        }
    }
    
    // Gather all results to rank 0
    int *recv_counts = nullptr;
    int *displs = nullptr;
    
    if (rank == 0) {
        recv_counts = new int[size];
        displs = new int[size];
        
        for (int i = 0; i < size; i++) {
            int i_start = i * (MAP_SIZE / size);
            int i_end = (i == size - 1) ? MAP_SIZE : i_start + (MAP_SIZE / size);
            recv_counts[i] = (i_end - i_start) * MAP_SIZE;
            displs[i] = i_start * MAP_SIZE;
        }
    }
    
    MPI_Gatherv(local_map, local_rows * MAP_SIZE, MPI_DOUBLE,
                global_map, recv_counts, displs, MPI_DOUBLE,
                0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        delete[] recv_counts;
        delete[] displs;
    }
    
    double end_time = MPI_Wtime();
    return end_time - start_time;
}

double ShockSimulation::runAsynchronous() {
    double start_time = MPI_Wtime();
    
    // Asynchronous execution: non-blocking communication
    // Use computation/communication overlap
    
    for (int time_step = 0; time_step < TIME_STEPS; time_step++) {
        // Compute local region (no barrier)
        computeLocalRegion(time_step);
        
        // Non-blocking progress report (avoid synchronization)
        if (rank == 0 && (time_step + 1) % 10 == 0) {
            std::cout << "Asynchronous: Step " << (time_step + 1) 
                      << "/" << TIME_STEPS << " completed" << std::endl;
        }
        
        // Continue immediately to next iteration without waiting
        // (processes work independently)
    }
    
    // Final gather (only synchronization point)
    int *recv_counts = nullptr;
    int *displs = nullptr;
    
    if (rank == 0) {
        recv_counts = new int[size];
        displs = new int[size];
        
        for (int i = 0; i < size; i++) {
            int i_start = i * (MAP_SIZE / size);
            int i_end = (i == size - 1) ? MAP_SIZE : i_start + (MAP_SIZE / size);
            recv_counts[i] = (i_end - i_start) * MAP_SIZE;
            displs[i] = i_start * MAP_SIZE;
        }
    }
    
    MPI_Gatherv(local_map, local_rows * MAP_SIZE, MPI_DOUBLE,
                global_map, recv_counts, displs, MPI_DOUBLE,
                0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        delete[] recv_counts;
        delete[] displs;
    }
    
    double end_time = MPI_Wtime();
    return end_time - start_time;
}

void ShockSimulation::saveToCSV(const char* filename) {
    if (rank != 0) return; // Only rank 0 writes
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }
    
    std::cout << "Writing results to " << filename << "..." << std::flush;
    
    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            file << global_map[i * MAP_SIZE + j];
            if (j < MAP_SIZE - 1) file << ",";
        }
        file << "\n";
    }
    
    file.close();
    std::cout << " Done!" << std::endl;
}

