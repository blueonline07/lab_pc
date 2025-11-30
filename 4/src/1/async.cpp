#include "common.h"
#include <mpi.h>
#include <vector>

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double kernel[3][3] = {
        {0.05, 0.1, 0.05},
        {0.1, 0.4, 0.1},
        {0.05, 0.1, 0.05},
    };

    double **grid = new double *[N + 2];
    double **new_grid = new double *[N + 2];
    for (int i = 0; i <= N + 1; i++)
    {
        grid[i] = new double[N + 2];
        new_grid[i] = new double[N + 2];
        for (int j = 0; j <= N + 1; j++)
        {
            grid[i][j] = 0.0;
            new_grid[i][j] = 0.0;
        }
    }

    // Only rank 0 reads the file
    if (rank == 0)
    {
        if (!read_file(grid, argv[1]))
        {
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
    }

    double t0 = MPI_Wtime();
    // Broadcast grid to all processes
    for (int i = 0; i <= N + 1; i++)
    {
        MPI_Bcast(grid[i], N + 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }

    std::vector<MPI_Request> requests;

    for (int t = 0; t < NUM_ITERS; t++)
    {
        requests.clear();

        // Compute this process's rows
        for (int i = 1 + rank; i <= N; i += size)
        {
            for (int j = 1; j <= N; j++)
            {
                double sum = 0;
                for (int ki = 0; ki < 3; ki++)
                {
                    for (int kj = 0; kj < 3; kj++)
                    {
                        int ni = i + ki - 1;
                        int nj = j + kj - 1;
                        sum += grid[ni][nj] * kernel[ki][kj];
                    }
                }
                new_grid[i][j] = sum;
            }

            // Asynchronously broadcast this row to all other processes
            MPI_Request req;
            MPI_Ibcast(&new_grid[i][1], N, MPI_DOUBLE, rank, MPI_COMM_WORLD, &req);
            requests.push_back(req);
        }

        // Asynchronously receive rows from other processes
        for (int i = 1; i <= N; i++)
        {
            int owner = (i - 1) % size;
            if (owner != rank)
            {
                MPI_Request req;
                MPI_Ibcast(&new_grid[i][1], N, MPI_DOUBLE, owner, MPI_COMM_WORLD, &req);
                requests.push_back(req);
            }
        }

        // Wait for all communications to complete
        MPI_Waitall(requests.size(), requests.data(), MPI_STATUSES_IGNORE);

        // Swap grids
        double **temp = grid;
        grid = new_grid;
        new_grid = temp;
    }

    if (rank == 0)
        std::cout << MPI_Wtime() - t0 << std::endl;
    // Cleanup
    for (int i = 0; i <= N + 1; i++)
    {
        delete[] grid[i];
        delete[] new_grid[i];
    }
    delete[] grid;
    delete[] new_grid;

    MPI_Finalize();
    return 0;
}