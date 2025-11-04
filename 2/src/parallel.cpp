#include "simulation.h"
#include <mpi.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

int main(int argc, char *argv[])
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Validate that grid size is divisible by number of processes
    if (GRID_SIZE % size != 0)
    {
        if (rank == 0)
        {
            std::cerr << "Error: Grid size (" << GRID_SIZE
                      << ") must be divisible by number of processes (" << size << ")" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    if (rank == 0)
    {
        std::cout << "=== Parallel Contamination Diffusion Simulation ===" << std::endl;
        std::cout << "Number of processes: " << size << std::endl;
        std::cout << "Grid Size: " << GRID_SIZE << "x" << GRID_SIZE << std::endl;
        std::cout << "Simulation Time: " << SIMULATION_TIME << " seconds" << std::endl;
        std::cout << "Wind Velocity: (" << WIND_X << ", " << WIND_Y << ") m/s" << std::endl;
        std::cout << "Diffusion Coefficient: " << DIFFUSION_COEFF << std::endl;
        std::cout << "Decay Rate: " << DECAY_RATE << std::endl;
        std::cout << "Deposition Rate: " << DEPOSITION_RATE << std::endl;
        std::cout << std::endl;
    }

    // Calculate local grid dimensions
    int local_rows = GRID_SIZE / size;
    int local_cols = GRID_SIZE;

    // Create local simulation
    ContaminationSimulation local_sim(local_rows + 2, local_cols); // +2 for ghost cells

    // Create temporary grid for computation (like sequential version)
    std::vector<std::vector<double>> temp_grid(local_rows + 2, std::vector<double>(local_cols, 0.0));

    // Initialize global grid on process 0
    std::vector<std::vector<double>> global_grid;
    if (rank == 0)
    {
        global_grid.resize(GRID_SIZE, std::vector<double>(GRID_SIZE, 0.0));
        global_grid[INITIAL_X][INITIAL_Y] = INITIAL_CONTAMINATION;

        std::cout << "Initial contamination at center: " << INITIAL_CONTAMINATION << std::endl;
    }

    // Scatter initial grid to all processes
    std::vector<double> send_buffer, recv_buffer;
    if (rank == 0)
    {
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

    // Copy received data to local grid (excluding ghost cells)
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
        local_sim.getGrid()[0][j] = 0.0;              // Top ghost row
        local_sim.getGrid()[local_rows + 1][j] = 0.0; // Bottom ghost row
    }

    // Count initial uncontaminated blocks (consistent with computation loop)
    int local_uncontaminated = 0;
    for (int i = 1; i < local_rows + 1; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            if (local_sim.getGrid()[i][j] < CONTAMINATION_THRESHOLD)
            {
                local_uncontaminated++;
            }
        }
    }

    int global_uncontaminated;
    MPI_Reduce(&local_uncontaminated, &global_uncontaminated, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        std::cout << "Initial uncontaminated blocks: " << global_uncontaminated << std::endl;
        std::cout << std::endl;
    }

    // Start timing
    double start_time = MPI_Wtime();

    // Run simulation
    for (int step = 0; step < SIMULATION_TIME; step++)
    {
        // Exchange boundary data with neighboring processes
        if (rank > 0)
        {
            // Send top row to previous process, receive from previous process
            MPI_Sendrecv(&local_sim.getGrid()[1][0], GRID_SIZE, MPI_DOUBLE, rank - 1, 0,
                         &local_sim.getGrid()[0][0], GRID_SIZE, MPI_DOUBLE, rank - 1, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        if (rank < size - 1)
        {
            // Send bottom row to next process, receive from next process
            MPI_Sendrecv(&local_sim.getGrid()[local_rows][0], GRID_SIZE, MPI_DOUBLE, rank + 1, 0,
                         &local_sim.getGrid()[local_rows + 1][0], GRID_SIZE, MPI_DOUBLE, rank + 1, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // Compute local update using finite differences (excluding boundaries)
        for (int i = 1; i < local_rows; i++)
        {
            for (int j = 1; j < GRID_SIZE - 1; j++)
            {
                double advection_x = 0.0, advection_y = 0.0;
                double diffusion = 0.0;

                // Advection (upwind scheme with negative sign for correct physics)
                if (i > 0)
                {
                    advection_x = WIND_X * (local_sim.getGrid()[i][j] - local_sim.getGrid()[i - 1][j]) / DX;
                }
                if (j > 0)
                {
                    advection_y = WIND_Y * (local_sim.getGrid()[i][j] - local_sim.getGrid()[i][j - 1]) / DY;
                }

                // Diffusion (central difference)
                diffusion = DIFFUSION_COEFF * ((local_sim.getGrid()[i + 1][j] - 2 * local_sim.getGrid()[i][j] + local_sim.getGrid()[i - 1][j]) / (DX * DX) +
                                               (local_sim.getGrid()[i][j + 1] - 2 * local_sim.getGrid()[i][j] + local_sim.getGrid()[i][j - 1]) / (DY * DY));

                // Decay
                double decay = (DECAY_RATE + DEPOSITION_RATE) * local_sim.getGrid()[i][j];

                // Forward Euler time integration
                double new_value = local_sim.getGrid()[i][j] + TIME_STEP * (-(advection_x + advection_y) + diffusion - decay);
                temp_grid[i][j] = std::max(0.0, new_value);
            }
        }

        // Swap grids (like sequential version)
        local_sim.getGrid().swap(temp_grid);

        // Apply boundary conditions after computation (consistent with sequential version)
        if (rank == 0)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                local_sim.getGrid()[1][j] = 0.0; // Top boundary (first actual row)
            }
        }
        if (rank == size - 1)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                local_sim.getGrid()[local_rows][j] = 0.0; // Bottom boundary (last actual row)
            }
        }

        // Apply left and right boundaries
        for (int i = 1; i < local_rows + 1; i++)
        {
            local_sim.getGrid()[i][0] = 0.0;             // Left boundary
            local_sim.getGrid()[i][GRID_SIZE - 1] = 0.0; // Right boundary
        }

        // Synchronize all processes
        MPI_Barrier(MPI_COMM_WORLD);

        // Count uncontaminated blocks (consistent with computation loop)
        local_uncontaminated = 0;
        for (int i = 1; i < local_rows + 1; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                if (local_sim.getGrid()[i][j] < CONTAMINATION_THRESHOLD)
                {
                    local_uncontaminated++;
                }
            }
        }

        MPI_Reduce(&local_uncontaminated, &global_uncontaminated, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        // Report progress every 10 steps
        if (rank == 0 && (step % 10 == 0 || step == SIMULATION_TIME - 1))
        {
            std::cout << "Time Step " << std::setw(3) << step
                      << ": Uncontaminated blocks = "
                      << std::setw(8) << global_uncontaminated << std::endl;
        }
    }

    // Calculate total execution time
    double end_time = MPI_Wtime();
    double execution_time = end_time - start_time;

    // Gather final results
    std::vector<double> gather_buffer;
    if (rank == 0)
    {
        gather_buffer.resize(GRID_SIZE * GRID_SIZE);
    }

    // Prepare local data for gathering (excluding ghost cells)
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

    // Print final results on process 0
    if (rank == 0)
    {
        // Reconstruct global grid
        for (int i = 0; i < GRID_SIZE; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                global_grid[i][j] = gather_buffer[i * GRID_SIZE + j];
            }
        }

        std::cout << "\n=== Simulation Results ===" << std::endl;
        std::cout << "Grid Size: " << GRID_SIZE << "x" << GRID_SIZE << std::endl;
        std::cout << "Total Blocks: " << GRID_SIZE * GRID_SIZE << std::endl;
        std::cout << "Uncontaminated Blocks: " << global_uncontaminated << std::endl;

        // Calculate total contamination
        double total_contamination = 0.0;
        for (int i = 0; i < GRID_SIZE; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                total_contamination += global_grid[i][j];
            }
        }
        std::cout << "Total Contamination: " << std::fixed << std::setprecision(6)
                  << total_contamination << std::endl;

        std::cout << "Execution Time: " << std::fixed << std::setprecision(3)
                  << execution_time << " seconds" << std::endl;

        // Print center region
        std::cout << "\nCenter Region (20x20 around contamination source):" << std::endl;
        int center_i = INITIAL_X;
        int center_j = INITIAL_Y;
        int half_size = 10;

        for (int i = std::max(0, center_i - half_size);
             i < std::min(GRID_SIZE, center_i + half_size); i++)
        {
            for (int j = std::max(0, center_j - half_size);
                 j < std::min(GRID_SIZE, center_j + half_size); j++)
            {
                std::cout << std::setw(8) << std::fixed << std::setprecision(2)
                          << global_grid[i][j] << " ";
            }
            std::cout << std::endl;
        }

        std::cout << "\nParallel simulation completed successfully!" << std::endl;
    }

    MPI_Finalize();
    return 0;
}
