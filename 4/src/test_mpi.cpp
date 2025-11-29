#include <mpi.h>
#include <iostream>
#include <unistd.h>
#include <cstring>

int main(int argc, char** argv) {
    // Initialize MPI
    MPI_Init(&argc, &argv);
    
    int rank, size;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    
    // Get process rank and total number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Get_processor_name(processor_name, &name_len);
    
    // Simple computation to verify MPI is working
    int local_value = rank * 10;
    int global_sum = 0;
    
    // Perform a reduction operation to test communication
    MPI_Reduce(&local_value, &global_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    // Print information from each process
    std::cout << "Process " << rank << " of " << size 
              << " running on node: " << processor_name 
              << " (PID: " << getpid() << ")" << std::endl;
    
    // Root process prints the result
    if (rank == 0) {
        std::cout << "\n=== MPI Test Results ===" << std::endl;
        std::cout << "Total processes: " << size << std::endl;
        std::cout << "Sum of all ranks (0 to " << (size-1) << ") * 10 = " 
                  << global_sum << std::endl;
        std::cout << "Expected sum: " << (size * (size - 1) / 2 * 10) << std::endl;
        std::cout << "Test " << (global_sum == (size * (size - 1) / 2 * 10) ? "PASSED" : "FAILED") 
                  << std::endl;
    }
    
    // Synchronize all processes
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Finalize MPI
    MPI_Finalize();
    
    return 0;
}

