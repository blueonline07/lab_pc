#include "simulation.h"
#include "arg_parser.h"
#include <mpi.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <algorithm>

int main(int argc, char *argv[])
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    ProgramOptions opts = parse_arguments(argc, argv, argv[0], true);

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

    int local_rows = GRID_SIZE / size;
    int local_cols = GRID_SIZE;

    ContaminationSimulation local_sim(local_rows + 2, local_cols);
    std::vector<std::vector<double>> temp_grid(local_rows + 2, std::vector<double>(local_cols, 0.0));

    std::vector<double> send_buffer, recv_buffer;
    if (rank == 0)
    {
        ContaminationSimulation global_sim(GRID_SIZE, GRID_SIZE);
        global_sim.initializeFromFile(opts.input_file);
        
        send_buffer.resize(GRID_SIZE * GRID_SIZE);
        const auto& grid = global_sim.getGrid();
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                send_buffer[i * GRID_SIZE + j] = grid[i][j];
            }
        }
    }

    recv_buffer.resize(local_rows * GRID_SIZE);
    MPI_Scatter(send_buffer.data(), local_rows * GRID_SIZE, MPI_DOUBLE,
                recv_buffer.data(), local_rows * GRID_SIZE, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    for (int i = 0; i < local_rows; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            local_sim.getGrid()[i + 1][j] = recv_buffer[i * GRID_SIZE + j];
        }
    }

    for (int j = 0; j < GRID_SIZE; j++)
    {
        local_sim.getGrid()[0][j] = 0.0;
        local_sim.getGrid()[local_rows + 1][j] = 0.0;
    }

    double start_time = MPI_Wtime();

    for (int step = 0; step < SIMULATION_TIME; step++)
    {
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

        for (int i = 1; i <= local_rows; i++)
        {
            for (int j = 1; j < GRID_SIZE - 1; j++)
            {
                double advection_x = 0.0, advection_y = 0.0;
                double diffusion = 0.0;

                if (i > 0) {
                    advection_x = WIND_X * (local_sim.getGrid()[i][j] - local_sim.getGrid()[i - 1][j]) / DX;
                }
                if (j > 0) {
                    advection_y = WIND_Y * (local_sim.getGrid()[i][j] - local_sim.getGrid()[i][j - 1]) / DY;
                }

                const auto& g = local_sim.getGrid();
                diffusion = DIFFUSION_COEFF * (
                    (g[i + 1][j] - 2 * g[i][j] + g[i - 1][j]) / (DX * DX) +
                    (g[i][j + 1] - 2 * g[i][j] + g[i][j - 1]) / (DY * DY)
                );

                double decay = (DECAY_RATE + DEPOSITION_RATE) * g[i][j];

                temp_grid[i][j] = std::max(0.0, 
                    g[i][j] + TIME_STEP * (-(advection_x + advection_y) + diffusion - decay));
            }
        }

        local_sim.getGrid().swap(temp_grid);

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

    int stop_signal = 1;
    MPI_Bcast(&stop_signal, 1, MPI_INT, 0, MPI_COMM_WORLD);

    std::vector<double> gather_buffer;
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

    if (rank == 0)
    {
        std::cout << "Final assembly complete." << std::endl;
    }

    double end_time = MPI_Wtime();
    double execution_time = end_time - start_time;

    if (rank == 0)
    {
        std::cout << std::fixed << std::setprecision(3)
                  << "Parallel: " << execution_time << " s\n";
        
        if (opts.save_output) {
            std::ofstream file("lab2_parallel_result.csv");
            if (file.is_open()) {
                std::cout << "Writing results to lab2_parallel_result.csv..." << std::flush;
                for (int i = 0; i < GRID_SIZE; i++) {
                    for (int j = 0; j < GRID_SIZE; j++) {
                        file << gather_buffer[i * GRID_SIZE + j] << (j < GRID_SIZE - 1 ? "," : "");
                    }
                    file << "\n";
                }
                std::cout << " Done!" << std::endl;
            } else {
                std::cerr << "Error: Could not open file lab2_parallel_result.csv for writing." << std::endl;
            }
        } else {
            std::cout << "CSV output skipped.\n";
        }
    }

    MPI_Finalize();
    return 0;
}
