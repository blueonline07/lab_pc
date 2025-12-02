#include "common.h"

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    double t0 = MPI_Wtime();
    int rank = -1, size = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
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

    for (int t = 0; t < SIMULATION_STEPS; t++)
    {
        MPI_Request req[4];
        int req_count = 0;
        double *prev = new double[N];
        double *next = new double[N];
        int uncontaminated = 0;
        int total_uncontaminated = 0;
        if (rank != 0)
            MPI_Irecv(prev, N, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &req[req_count++]);
        if (rank != size - 1)
            MPI_Irecv(next, N, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &req[req_count++]);

        if (rank != 0)
            MPI_Isend(local, N, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &req[req_count++]);
        if (rank != size - 1)
            MPI_Isend(&local[(N / size - 1) * N], N, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &req[req_count++]);

        MPI_Waitall(req_count, req, MPI_STATUSES_IGNORE);
        for (int i = 0; i < N / size; i++)
        {
            for (int j = 0; j < N; j++)
            {
                double n, s;
                if (i > 0)
                {
                    n = local[(i - 1) * N + j];
                }
                else
                {
                    n = (rank != 0) ? prev[j] : 0.0;
                }

                if (i < N / size - 1)
                {
                    s = local[(i + 1) * N + j];
                }
                else
                {
                    s = (rank != size - 1) ? next[j] : 0.0;
                }

                double w = j > 0 ? local[i * N + (j - 1)] : 0;
                double e = j < N - 1 ? local[i * N + (j + 1)] : 0;
                double cur = local[i * N + j];

                double advection = WIND_X * (cur - n) / DX + WIND_Y * (cur - w) / DY;
                double diffusion = DIFFUSION_COEFF * (s - 2 * cur + n) / (DX * DX) + DIFFUSION_COEFF * (e - 2 * cur + w) / (DY * DY);
                double decay = DECAY_RATE * cur + DEPOSITION_RATE * cur;
                cur = cur + TIME_STEP * (-advection + diffusion - decay);
                temp[i * N + j] = std::max(0.0, cur);
                if (temp[i * N + j] == 0)
                    uncontaminated++;
            }
        }

        delete[] prev;
        delete[] next;
        std::swap(local, temp);
        MPI_Reduce(&uncontaminated, &total_uncontaminated, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        if (rank == 0)
            std::cout << total_uncontaminated << std::endl;
    }
    MPI_Gather(local, chunk, MPI_DOUBLE, (rank == 0 ? grid : nullptr), chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    delete[] grid;
    if (rank == 0)
        std::cout << "Parallel: " << MPI_Wtime() - t0;
    MPI_Finalize();
    return 0;
}