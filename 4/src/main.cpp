#include <mpi.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <unistd.h>
#include "shock_simulation.h"

void print_header(int rank, int size) {
    if (rank == 0) {
        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "  Multi-Host MPI Shock Wave Simulation\n";
        std::cout << "========================================\n";
        std::cout << "Total MPI processes: " << size << "\n";
        std::cout << "Grid size: 4000 x 4000\n";
        std::cout << "Time steps: 100\n";
        std::cout << "Yield: 5000 kt\n";
        std::cout << "========================================\n";
        std::cout << "\n";
    }
}

void print_host_info(int rank, int size) {
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    
    // Print in order by rank
    for (int i = 0; i < size; i++) {
        if (rank == i) {
            std::cout << "  Process " << rank << " running on: " << hostname << std::endl;
            std::cout.flush();
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Parse command line arguments
    bool save_output = true;
    bool run_sync = true;
    bool run_async = true;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--no-output" || arg == "--skip-csv") {
            save_output = false;
        } else if (arg == "--sync-only") {
            run_async = false;
        } else if (arg == "--async-only") {
            run_sync = false;
        } else if (arg == "--help" || arg == "-h") {
            if (rank == 0) {
                std::cout << "Usage: mpirun -np <procs> --hostfile hosts.txt " << argv[0] 
                          << " [options]\n"
                          << "Options:\n"
                          << "  --no-output, --skip-csv    Skip CSV output generation\n"
                          << "  --sync-only                Run only synchronous version\n"
                          << "  --async-only               Run only asynchronous version\n"
                          << "  --help, -h                 Show this help message\n";
            }
            MPI_Finalize();
            return 0;
        }
    }
    
    // Print header and host information
    print_header(rank, size);
    if (rank == 0) {
        std::cout << "Process distribution:\n";
    }
    print_host_info(rank, size);
    
    if (rank == 0) {
        std::cout << "\n========================================\n\n";
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Create simulation
    ShockSimulation simulation(rank, size);
    
    double sync_time = 0.0;
    double async_time = 0.0;
    
    // Run synchronous version
    if (run_sync) {
        if (rank == 0) {
            std::cout << "Running SYNCHRONOUS simulation...\n";
        }
        sync_time = simulation.runSynchronous();
        
        if (rank == 0) {
            std::cout << std::fixed << std::setprecision(3);
            std::cout << "Synchronous execution time: " << sync_time << " s\n";
            
            if (save_output) {
                simulation.saveToCSV("lab4_synchronous_result.csv");
            }
        }
        
        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == 0) std::cout << "\n";
    }
    
    // Run asynchronous version
    if (run_async) {
        if (rank == 0) {
            std::cout << "Running ASYNCHRONOUS simulation...\n";
        }
        async_time = simulation.runAsynchronous();
        
        if (rank == 0) {
            std::cout << std::fixed << std::setprecision(3);
            std::cout << "Asynchronous execution time: " << async_time << " s\n";
            
            if (save_output) {
                simulation.saveToCSV("lab4_asynchronous_result.csv");
            }
        }
    }
    
    // Print comparison
    if (rank == 0 && run_sync && run_async) {
        std::cout << "\n========================================\n";
        std::cout << "Performance Comparison:\n";
        std::cout << "========================================\n";
        std::cout << "Synchronous:  " << sync_time << " s\n";
        std::cout << "Asynchronous: " << async_time << " s\n";
        double speedup = sync_time / async_time;
        double improvement = ((sync_time - async_time) / sync_time) * 100.0;
        std::cout << "Speedup:      " << speedup << "x\n";
        std::cout << "Improvement:  " << improvement << "%\n";
        std::cout << "========================================\n";
    }
    
    MPI_Finalize();
    return 0;
}
