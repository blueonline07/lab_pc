#include "common.h"
#include <mpi.h>
#include <chrono>

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    double t0 = MPI_Wtime();
    int rank = -1, size = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    double k[3][3] = {
        {0.05, 0.1, 0.05},
        {0.1, 0.4, 0.1},
        {0.05, 0.1, 0.05},
    };
    double *grid = new double[N * N];
    if (rank == 0)
    {
        std::ifstream file(argv[1]);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file " << argv[1] << std::endl;
            return 1;
        }
        std::string line;
        for (int i = 0; i < N; i++)
        {
            if (!std::getline(file, line))
            {
                return 1;
            }
            std::istringstream ss(line);
            for (int j = 0; j < N; j++)
            {
                std::string token;
                if (!std::getline(ss, token, ','))
                {
                    return 1;
                }
                grid[i * N + j] = std::stod(token);
            }
        }
        file.close();
    }

    int chunk = (N / size) * N;
    double *local = new double[chunk];
    double *temp = new double[chunk];

    MPI_Scatter((rank == 0 ? grid : nullptr), chunk, MPI_DOUBLE, local, chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    for (int t = 0; t < NUM_ITERS; t++)
    {
        double *prev = new double[N];
        double *next = new double[N];
        if (rank != 0 && rank != size - 1)
        {

            MPI_Sendrecv(local, N, MPI_DOUBLE, rank - 1, 0,
                         prev, N, MPI_DOUBLE, rank - 1, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            MPI_Sendrecv(&local[(N / size - 1) * N], N, MPI_DOUBLE, rank + 1, 0,
                         next, N, MPI_DOUBLE, rank + 1, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else if (rank == 0)
        {

            MPI_Sendrecv(&local[(N / size - 1) * N], N, MPI_DOUBLE, rank + 1, 0,
                         next, N, MPI_DOUBLE, rank + 1, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else if (rank == size - 1)
        {

            MPI_Sendrecv(local, N, MPI_DOUBLE, rank - 1, 0,
                         prev, N, MPI_DOUBLE, rank - 1, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        for (int i = 0; i < N / size; i++)
        {
            for (int j = 0; j < N; j++)
            {
                double n, s, ne, nw, se, sw;
                if (i > 0)
                {
                    n = local[(i - 1) * N + j];
                }
                else
                {
                    n = (rank != 0) ? prev[j] : 30.0;
                }

                if (i < N / size - 1)
                {
                    s = local[(i + 1) * N + j];
                }
                else
                {
                    s = (rank != size - 1) ? next[j] : 30.0;
                }

                if (i > 0 && j > 0)
                {
                    nw = local[(i - 1) * N + (j - 1)];
                }
                else
                {
                    nw = (rank != 0 && j > 0) ? prev[j - 1] : 30.0;
                }

                if (i > 0 && j < N - 1)
                {
                    ne = local[(i - 1) * N + (j + 1)];
                }
                else
                {
                    ne = (rank != 0 && j < N - 1) ? prev[j + 1] : 30.0;
                }

                if (i < N / size - 1 && j > 0)
                {
                    sw = local[(i + 1) * N + (j - 1)];
                }
                else
                {
                    sw = (rank != size - 1 && j > 0) ? next[j - 1] : 30.0;
                }

                if (i < N / size - 1 && j < N - 1)
                {
                    se = local[(i + 1) * N + (j + 1)];
                }
                else
                {
                    se = (rank != size - 1 && j < N - 1) ? next[j + 1] : 30.0;
                }

                double w = (j > 0) ? local[i * N + j - 1] : 30.0;
                double e = (j < N - 1) ? local[i * N + j + 1] : 30.0;
                temp[i * N + j] = nw * k[0][0] + n * k[0][1] + ne * k[0][2] + w * k[1][0] + local[i * N + j] * k[1][1] + e * k[1][2] + sw * k[2][0] + s * k[2][1] + se * k[2][2];
            }
        }

        delete[] prev;
        delete[] next;
        std::swap(local, temp);
    }
    MPI_Gather(local, chunk, MPI_DOUBLE, (rank == 0 ? grid : nullptr), chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    delete[] grid;
    if (rank == 0)
        std::cout << "Parallel: " << MPI_Wtime() - t0;
    MPI_Finalize();
    return 0;
}
