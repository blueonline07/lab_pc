#include "common.h"

int sq(int x)
{
    return x * x;
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const double c[9] = {2.611369, -1.690128, 0.00805, 0.336743, -0.005162, -0.080923, -0.004785, 0.007930, 0.000768};

    // Calculate workload distribution
    int rows_per_proc = N / size;
    int remainder = N % size;
    int start_row = rank * rows_per_proc + std::min(rank, remainder);
    int end_row = start_row + rows_per_proc + (rank < remainder ? 1 : 0);
    int local_rows = end_row - start_row;

    // Allocate local grid
    double **local_grid = new double *[local_rows];
    for (int i = 0; i < local_rows; i++)
    {
        local_grid[i] = new double[N];
        for (int j = 0; j < N; j++)
        {
            local_grid[i][j] = 0.0;
        }
    }

    // Full grid only on rank 0 for final result
    double **grid = nullptr;
    std::vector<MPI_Request> requests;

    if (rank == 0)
    {
        grid = new double *[N];
        for (int i = 0; i < N; i++)
        {
            grid[i] = new double[N];
        }
        // Pre-allocate requests for non-blocking receives
        requests.resize(size * local_rows);
    }

    double start = MPI_Wtime();

    // Simulate TIME steps
    for (int t = 0; t < TIME; t++)
    {
        // Each process computes its assigned rows asynchronously
        // No barrier - processes work independently
        for (int i = 0; i < local_rows; i++)
        {
            int global_i = start_row + i;
            for (int j = 0; j < N; j++)
            {
                double R = sqrt(sq(global_i - CENTER_X) + sq(j - CENTER_Y)) * CELL_SIZE;

                if (t >= R / 343.0)
                {
                    double Z = R * pow(W, -1.0 / 3.0);
                    double U = -0.21436 + 1.35034 * log10(Z);
                    double log10P = 0.0;
                    for (int k = 0; k < 9; k++)
                    {
                        log10P += c[k] * pow(U, k);
                    }
                    local_grid[i][j] = pow(10.0, log10P);
                }
            }
        }
        // No MPI_Barrier here - asynchronous execution
    }

    // Non-blocking gather results at rank 0
    if (rank == 0)
    {
        // Copy local data
        for (int i = 0; i < local_rows; i++)
        {
            for (int j = 0; j < N; j++)
            {
                grid[start_row + i][j] = local_grid[i][j];
            }
        }

        // Post non-blocking receives from other processes
        int req_idx = 0;
        for (int p = 1; p < size; p++)
        {
            int p_rows_per_proc = N / size;
            int p_remainder = N % size;
            int p_start_row = p * p_rows_per_proc + std::min(p, p_remainder);
            int p_local_rows = p_rows_per_proc + (p < p_remainder ? 1 : 0);

            for (int i = 0; i < p_local_rows; i++)
            {
                MPI_Irecv(grid[p_start_row + i], N, MPI_DOUBLE, p, i,
                          MPI_COMM_WORLD, &requests[req_idx++]);
            }
        }

        // Wait for all non-blocking receives to complete
        MPI_Waitall(req_idx, requests.data(), MPI_STATUSES_IGNORE);
    }
    else
    {
        // Non-blocking sends to rank 0
        std::vector<MPI_Request> send_requests(local_rows);

        for (int i = 0; i < local_rows; i++)
        {
            MPI_Isend(local_grid[i], N, MPI_DOUBLE, 0, i,
                      MPI_COMM_WORLD, &send_requests[i]);
        }

        // Wait for all sends to complete
        MPI_Waitall(local_rows, send_requests.data(), MPI_STATUSES_IGNORE);
    }

    if (rank == 0)
    {
        std::cout << "Asynchronous MPI Time: "
                  << MPI_Wtime() - start
                  << " seconds" << std::endl;
    }

    // Cleanup
    for (int i = 0; i < local_rows; i++)
    {
        delete[] local_grid[i];
    }
    delete[] local_grid;

    if (rank == 0 && grid != nullptr)
    {
        for (int i = 0; i < N; i++)
        {
            delete[] grid[i];
        }
        delete[] grid;
    }

    MPI_Finalize();
    return 0;
}