#include "simulation.h"

void simulate(double *grid, int rows_per_process, int rank, int size)
{

    for (int t = 0; t < SIMULATION_STEPS; t++)
    {
        if (rank > 0)
        {
            MPI_Sendrecv(
                &grid[1 * (GRID_SIZE + 2)], GRID_SIZE + 2, MPI_DOUBLE, rank - 1, 0,
                &grid[0], GRID_SIZE + 2, MPI_DOUBLE, rank - 1, 0,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        if (rank < size - 1)
        {
            int last_data_row = rows_per_process * (GRID_SIZE + 2);
            int bottom_ghost_row = (rows_per_process + 1) * (GRID_SIZE + 2);
            MPI_Sendrecv(
                &grid[last_data_row], GRID_SIZE + 2, MPI_DOUBLE, rank + 1, 1,
                &grid[bottom_ghost_row], GRID_SIZE + 2, MPI_DOUBLE, rank + 1, 1,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        for (int i = 1; i <= rows_per_process; i++)
        {
            for (int j = 1; j <= GRID_SIZE; j++)
            {
                int idx = i * (GRID_SIZE + 2) + j;
                int up_idx = (i - 1) * (GRID_SIZE + 2) + j;
                int down_idx = (i + 1) * (GRID_SIZE + 2) + j;
                int left_idx = i * (GRID_SIZE + 2) + (j - 1);
                int right_idx = i * (GRID_SIZE + 2) + (j + 1);

                double advection = WIND_X * (grid[idx] - grid[up_idx]) / DX + WIND_Y * (grid[idx] - grid[left_idx]) / DY;
                double diffusion = DIFFUSION_COEFF * (grid[down_idx] - 2 * grid[idx] + grid[up_idx]) / (DX * DX) + DIFFUSION_COEFF * (grid[right_idx] - 2 * grid[idx] + grid[left_idx]) / (DY * DY);
                double decay = DECAY_RATE * grid[idx] + DEPOSITION_RATE * grid[idx];
                grid[idx] = grid[idx] + TIME_STEP * (-advection + diffusion - decay);
                grid[idx] = std::max(0.0, grid[idx]);
            }
        }

        int local_uncontaminated = 0;
        for (int i = 1; i <= rows_per_process; i++)
        {
            for (int j = 1; j <= GRID_SIZE; j++)
            {
                int idx = i * (GRID_SIZE + 2) + j;
                if (grid[idx] < CONTAMINATION_THRESHOLD)
                {
                    local_uncontaminated++;
                }
            }
        }

        int total_uncontaminated = 0;
        MPI_Reduce(&local_uncontaminated, &total_uncontaminated, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        if (rank == 0)
        {
            std::cout << "Iteration " << t << ": " << total_uncontaminated << " uncontaminated blocks" << std::endl;
        }
    }
}

int main(int argc, char *argv[])
{
    double *grid = new double[(GRID_SIZE + 2) * (GRID_SIZE + 2)];
    for (int i = 0; i < (GRID_SIZE + 2) * (GRID_SIZE + 2); i++)
    {
        grid[i] = 0.0;
    }
    std::ifstream file(argv[1]);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file " << argv[1] << std::endl;
        return 1;
    }
    std::string line;
    for (int i = 1; i <= GRID_SIZE; i++)
    {
        if (!std::getline(file, line))
        {
            return 1;
        }
        std::istringstream ss(line);
        for (int j = 1; j <= GRID_SIZE; j++)
        {
            std::string token;
            if (!std::getline(ss, token, ','))
            {
                return 1;
            }
            grid[i * (GRID_SIZE + 2) + j] = std::stod(token);
        }
    }
    file.close();

    auto start_time = std::chrono::high_resolution_clock::now();
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rows_per_process = GRID_SIZE / size;

    double *local_grid = new double[(rows_per_process + 2) * (GRID_SIZE + 2)];

    for (int i = 0; i < (rows_per_process + 2) * (GRID_SIZE + 2); i++)
    {
        local_grid[i] = 0.0;
    }

    MPI_Scatter(&grid[1 * (GRID_SIZE + 2)], (GRID_SIZE + 2) * rows_per_process, MPI_DOUBLE,
                &local_grid[1 * (GRID_SIZE + 2)], (GRID_SIZE + 2) * rows_per_process, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    simulate(local_grid, rows_per_process, rank, size);

    MPI_Gather(&local_grid[1 * (GRID_SIZE + 2)], (GRID_SIZE + 2) * rows_per_process, MPI_DOUBLE,
               &grid[1 * (GRID_SIZE + 2)], (GRID_SIZE + 2) * rows_per_process, MPI_DOUBLE,
               0, MPI_COMM_WORLD);

    delete[] local_grid;
    auto end_time = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Parallel: " << duration << " ms" << std::endl;
    MPI_Finalize();
    return 0;
}