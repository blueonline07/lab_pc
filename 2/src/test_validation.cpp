#include "simulation.h"
#include <mpi.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <algorithm>

// Run sequential simulation and return final grid
std::vector<std::vector<double>> run_sequential()
{
    ContaminationSimulation sim(GRID_SIZE, GRID_SIZE);
    sim.initialize();

    for (int step = 0; step < SIMULATION_TIME; ++step)
    {
        sim.simulateStep();
    }

    return sim.getGrid();
}

// Run parallel simulation and return final grid
std::vector<std::vector<double>> run_parallel(int rank, int size)
{
    // Calculate local grid dimensions
    int local_rows = GRID_SIZE / size;
    int local_cols = GRID_SIZE;

    // Create local simulation with ghost cells
    ContaminationSimulation local_sim(local_rows + 2, local_cols);
    std::vector<std::vector<double>> temp_grid(local_rows + 2, std::vector<double>(local_cols, 0.0));

    // Initialize full global grid on process 0 first, then scatter
    std::vector<double> send_buffer, recv_buffer;
    std::vector<std::vector<double>> global_grid;
    if (rank == 0)
    {
        // Create and initialize the complete global grid
        global_grid.resize(GRID_SIZE, std::vector<double>(GRID_SIZE, 0.0));
        
        // Set initial contamination at the center
        global_grid[INITIAL_X][INITIAL_Y] = INITIAL_CONTAMINATION;
        
        // Flatten to send buffer
        send_buffer.resize(GRID_SIZE * GRID_SIZE);
        for (int i = 0; i < GRID_SIZE; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                send_buffer[i * GRID_SIZE + j] = global_grid[i][j];
            }
        }
    }

    recv_buffer.resize(local_rows * GRID_SIZE);
    MPI_Scatter(send_buffer.data(), local_rows * GRID_SIZE, MPI_DOUBLE,
                recv_buffer.data(), local_rows * GRID_SIZE, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    // Copy to local grid
    for (int i = 0; i < local_rows; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            local_sim.getGrid()[i + 1][j] = recv_buffer[i * GRID_SIZE + j];
        }
    }

    // Initialize ghost cells
    for (int j = 0; j < GRID_SIZE; j++)
    {
        local_sim.getGrid()[0][j] = 0.0;
        local_sim.getGrid()[local_rows + 1][j] = 0.0;
    }

    // Run simulation
    for (int step = 0; step < SIMULATION_TIME; step++)
    {
        // Exchange boundary data
        if (rank > 0)
        {
            MPI_Sendrecv(&local_sim.getGrid()[1][0], GRID_SIZE, MPI_DOUBLE, rank - 1, 0,
                         &local_sim.getGrid()[0][0], GRID_SIZE, MPI_DOUBLE, rank - 1, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        if (rank < size - 1)
        {
            MPI_Sendrecv(&local_sim.getGrid()[local_rows][0], GRID_SIZE, MPI_DOUBLE, rank + 1, 0,
                         &local_sim.getGrid()[local_rows + 1][0], GRID_SIZE, MPI_DOUBLE, rank + 1, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // Compute local update
        // Update all interior rows (1 to local_rows), boundaries will be overridden later
        for (int i = 1; i <= local_rows; i++)
        {
            for (int j = 1; j < GRID_SIZE - 1; j++)
            {
                double advection_x = 0.0, advection_y = 0.0;
                double diffusion = 0.0;

                if (i > 0)
                {
                    advection_x = WIND_X * (local_sim.getGrid()[i][j] - local_sim.getGrid()[i - 1][j]) / DX;
                }
                if (j > 0)
                {
                    advection_y = WIND_Y * (local_sim.getGrid()[i][j] - local_sim.getGrid()[i][j - 1]) / DY;
                }

                diffusion = DIFFUSION_COEFF * ((local_sim.getGrid()[i + 1][j] - 2 * local_sim.getGrid()[i][j] + local_sim.getGrid()[i - 1][j]) / (DX * DX) +
                                               (local_sim.getGrid()[i][j + 1] - 2 * local_sim.getGrid()[i][j] + local_sim.getGrid()[i][j - 1]) / (DY * DY));

                double decay = (DECAY_RATE + DEPOSITION_RATE) * local_sim.getGrid()[i][j];

                double new_value = local_sim.getGrid()[i][j] + TIME_STEP * (-(advection_x + advection_y) + diffusion - decay);
                temp_grid[i][j] = std::max(0.0, new_value);
            }
        }

        local_sim.getGrid().swap(temp_grid);

        // Apply boundary conditions
        if (rank == 0)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                local_sim.getGrid()[1][j] = 0.0;
            }
        }
        if (rank == size - 1)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                local_sim.getGrid()[local_rows][j] = 0.0;
            }
        }

        for (int i = 1; i < local_rows + 1; i++)
        {
            local_sim.getGrid()[i][0] = 0.0;
            local_sim.getGrid()[i][GRID_SIZE - 1] = 0.0;
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }

    // Gather final results
    std::vector<double> gather_buffer;
    if (rank == 0)
    {
        gather_buffer.resize(GRID_SIZE * GRID_SIZE);
    }

    std::vector<double> local_data(local_rows * GRID_SIZE);
    for (int i = 0; i < local_rows; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            local_data[i * GRID_SIZE + j] = local_sim.getGrid()[i + 1][j];
        }
    }

    MPI_Gather(local_data.data(), local_rows * GRID_SIZE, MPI_DOUBLE,
               gather_buffer.data(), local_rows * GRID_SIZE, MPI_DOUBLE,
               0, MPI_COMM_WORLD);

    // Reconstruct global grid
    if (rank == 0)
    {
        for (int i = 0; i < GRID_SIZE; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                global_grid[i][j] = gather_buffer[i * GRID_SIZE + j];
            }
        }
        return global_grid;
    }

    return std::vector<std::vector<double>>();
}

// Compare two grids
bool compare_grids(const std::vector<std::vector<double>> &a,
                   const std::vector<std::vector<double>> &b,
                   double tolerance = 1e-6)
{
    if (a.size() != b.size() || a[0].size() != b[0].size())
    {
        return false;
    }

    double max_diff = 0.0;
    int diff_count = 0;

    for (size_t i = 0; i < a.size(); i++)
    {
        for (size_t j = 0; j < a[0].size(); j++)
        {
            double diff = std::abs(a[i][j] - b[i][j]);
            if (diff > tolerance)
            {
                diff_count++;
                max_diff = std::max(max_diff, diff);
            }
        }
    }

    if (diff_count > 0)
    {
        std::cout << "Differences: " << diff_count << " cells" << std::endl;
        std::cout << "Max difference: " << max_diff << std::endl;
        return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (GRID_SIZE % size != 0)
    {
        if (rank == 0)
        {
            std::cerr << "Error: Grid size must be divisible by number of processes" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    if (rank == 0)
    {
        std::cout << "=== Contamination Diffusion Validation Test ===" << std::endl;
        std::cout << "Grid size: " << GRID_SIZE << "x" << GRID_SIZE << std::endl;
        std::cout << "Simulation time: " << SIMULATION_TIME << " steps" << std::endl;
        std::cout << "Number of processes: " << size << std::endl;
        std::cout << std::endl;
    }

    // Run sequential on rank 0
    std::vector<std::vector<double>> seq_grid;
    if (rank == 0)
    {
        std::cout << "Running sequential..." << std::endl;
        double t0 = MPI_Wtime();
        seq_grid = run_sequential();
        double t1 = MPI_Wtime();
        std::cout << "Sequential: " << std::fixed << std::setprecision(3)
                  << (t1 - t0) << " s" << std::endl;
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // Run parallel
    if (rank == 0)
    {
        std::cout << "Running parallel..." << std::endl;
    }

    double t0 = MPI_Wtime();
    std::vector<std::vector<double>> par_grid = run_parallel(rank, size);
    double t1 = MPI_Wtime();

    if (rank == 0)
    {
        std::cout << "Parallel:   " << std::fixed << std::setprecision(3)
                  << (t1 - t0) << " s" << std::endl;
        std::cout << std::endl;
    }

    // Validate on rank 0
    if (rank == 0)
    {
        std::cout << "=== Validation Results ===" << std::endl;
        std::cout << "Sequential vs Parallel: ";

        if (compare_grids(seq_grid, par_grid))
        {
            std::cout << "✓ PASS" << std::endl;
            std::cout << std::endl;
            std::cout << "Both implementations produce identical results!" << std::endl;
        }
        else
        {
            std::cout << "✗ FAIL" << std::endl;
            MPI_Finalize();
            return 1;
        }
    }

    MPI_Finalize();
    return 0;
}
