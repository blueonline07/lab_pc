#include <mpi.h>
#include <iostream>
#include <cstring>
#include <unistd.h>

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Get processor name (hostname)
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // Get hostname using system call
    char hostname[256];
    gethostname(hostname, sizeof(hostname));

    // Synchronize all processes
    MPI_Barrier(MPI_COMM_WORLD);

    // Print information from each process
    if (rank == 0)
    {
        std::cout << "=== MPI Multi-Host Execution ===" << std::endl;
        std::cout << "Total processes: " << size << std::endl;
        std::cout << "--------------------------------" << std::endl;
    }

    // Each process prints its information
    std::cout << "Process " << rank << " of " << size 
              << " running on host: " << hostname 
              << " (MPI name: " << processor_name << ")" << std::endl;

    // Synchronize before final message
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0)
    {
        std::cout << "--------------------------------" << std::endl;
        std::cout << "Execution completed successfully!" << std::endl;
    }

    MPI_Finalize();
    return 0;
}

