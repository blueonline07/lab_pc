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

    // Minimal output: suppress banners and configuration prints

    // Calculate local grid dimensions
    int local_rows = GRID_SIZE / size;
    int local_cols = GRID_SIZE;

    // Create local simulation
    ContaminationSimulation local_sim(local_rows + 2, local_cols); // +2 for ghost cells

    // Create temporary grid for computation (like sequential version)
    std::vector<std::vector<double>> temp_grid(local_rows + 2, std::vector<double>(local_cols, 0.0));

    // Initialize full global grid on process 0 first, then scatter
    std::vector<double> send_buffer, recv_buffer;
    if (rank == 0)
    {
        // Create and initialize the complete global grid
        std::vector<std::vector<double>> global_grid(GRID_SIZE, std::vector<double>(GRID_SIZE, 0.0));
        
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

    // Remove initial uncontaminated counting for minimal output

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

        // Compute local update using finite differences
        // Update all interior rows (1 to local_rows), boundaries will be overridden later
        for (int i = 1; i <= local_rows; i++)
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
        // Count uncontaminated blocks (exclude ghost rows and boundaries already zeroed)
        int local_uncontaminated = 0;
        for (int i = 1; i <= local_rows; ++i)
        {
            for (int j = 1; j < GRID_SIZE - 1; ++j)
            {
                double val = local_sim.getGrid()[i][j];
                if (val < CONTAMINATION_THRESHOLD)
                {
                    local_uncontaminated++;
                }
            }
        }
        int global_uncontaminated = 0;
        MPI_Reduce(&local_uncontaminated, &global_uncontaminated, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        if (rank == 0)
        {
            std::cout << "Step " << step + 1 << ": uncontaminated blocks = " << global_uncontaminated << '\n';
        }
    }

    // Broadcast stop signal after completing iterations (spec requirement)
    int stop_signal = 1;
    MPI_Bcast(&stop_signal, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Gather final local grids to rank 0 to assemble complete map
    std::vector<double> gather_buffer; // flattened global map
    if (rank == 0)
    {
        gather_buffer.resize(GRID_SIZE * GRID_SIZE);
    }

    std::vector<double> local_data(local_rows * GRID_SIZE);
    for (int i = 0; i < local_rows; ++i)
    {
        for (int j = 0; j < GRID_SIZE; ++j)
        {
            local_data[i * GRID_SIZE + j] = local_sim.getGrid()[i + 1][j];
        }
    }

    MPI_Gather(local_data.data(), local_rows * GRID_SIZE, MPI_DOUBLE,
               gather_buffer.data(), local_rows * GRID_SIZE, MPI_DOUBLE,
               0, MPI_COMM_WORLD);

    // Optionally report final uncontaminated blocks (redundant with last step but clearer)
    if (rank == 0)
    {
        std::cout << "Final assembly complete." << std::endl;
    }

    // Calculate total execution time
    double end_time = MPI_Wtime();
    double execution_time = end_time - start_time;

    // Minimal output: only print total time from rank 0
    if (rank == 0)
    {
        std::cout << std::fixed << std::setprecision(3)
                  << "Parallel: " << execution_time << " s\n";
    }

    MPI_Finalize();
    return 0;
}
