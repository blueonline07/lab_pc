#include <mpi.h>
#include <iostream>
#include <unistd.h>
#include <cstring>

int main(int argc, char** argv) {
    // Initialize MPI
    MPI_Init(&argc, &argv);

    int world_size, world_rank;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;

    // Get the number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Get the name of the processor
    MPI_Get_processor_name(processor_name, &name_len);

    // Print a hello world message
    std::cout << "Hello world from processor " << processor_name 
              << ", rank " << world_rank 
              << " out of " << world_size 
              << " processors" << std::endl;

    // Simple communication test: send and receive
    if (world_rank == 0) {
        int number = 42;
        std::cout << "Process 0 sending number " << number << " to process 1" << std::endl;
        MPI_Send(&number, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    } else if (world_rank == 1) {
        int number;
        MPI_Recv(&number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "Process 1 received number " << number << " from process 0" << std::endl;
    }

    // Barrier to synchronize all processes
    MPI_Barrier(MPI_COMM_WORLD);

    if (world_rank == 0) {
        std::cout << "\nAll processes synchronized successfully!" << std::endl;
    }

    // Finalize MPI
    MPI_Finalize();
    return 0;
}

