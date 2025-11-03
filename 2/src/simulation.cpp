#include "simulation.h"
#include <iostream>
#include <iomanip>

ContaminationSimulation::ContaminationSimulation(int rows, int cols)
    : rows(rows), cols(cols)
{
    // Initialize grids with proper dimensions
    grid.resize(rows, std::vector<double>(cols, 0.0));
    temp_grid.resize(rows, std::vector<double>(cols, 0.0));
}

void ContaminationSimulation::initialize()
{
    // Set initial contamination at center of grid
    if (INITIAL_X < rows && INITIAL_Y < cols)
    {
        grid[INITIAL_X][INITIAL_Y] = INITIAL_CONTAMINATION;
    }

    // Apply boundary conditions
    applyBoundaryConditions();
}

void ContaminationSimulation::applyBoundaryConditions()
{
    // Zero padding at boundaries
    for (int i = 0; i < rows; i++)
    {
        grid[i][0] = 0.0;        // Left boundary
        grid[i][cols - 1] = 0.0; // Right boundary
    }
    for (int j = 0; j < cols; j++)
    {
        grid[0][j] = 0.0;        // Top boundary
        grid[rows - 1][j] = 0.0; // Bottom boundary
    }
}

double ContaminationSimulation::computeAdvection(int i, int j)
{
    double advection_x = 0.0;
    double advection_y = 0.0;

    // Upwind scheme for advection (negative sign for correct physics)
    if (i > 0)
    {
        advection_x = -WIND_X * (grid[i][j] - grid[i - 1][j]) / CELL_EDGE;
    }
    if (j > 0)
    {
        advection_y = -WIND_Y * (grid[i][j] - grid[i][j - 1]) / CELL_EDGE;
    }

    return advection_x + advection_y;
}

double ContaminationSimulation::computeDiffusion(int i, int j)
{
    double diffusion = 0.0;

    // Central difference scheme for diffusion
    if (i > 0 && i < rows - 1)
    {
        diffusion += DIFFUSION_COEFF * (grid[i + 1][j] - 2 * grid[i][j] + grid[i - 1][j]) / (CELL_EDGE * CELL_EDGE);
    }
    if (j > 0 && j < cols - 1)
    {
        diffusion += DIFFUSION_COEFF * (grid[i][j + 1] - 2 * grid[i][j] + grid[i][j - 1]) / (CELL_EDGE * CELL_EDGE);
    }

    return diffusion;
}

double ContaminationSimulation::computeDecay(int i, int j)
{
    return (DECAY_RATE + DEPOSITION_RATE) * grid[i][j];
}

void ContaminationSimulation::simulateStep()
{
    // Apply PDE for one time step using finite differences
    for (int i = 1; i < rows - 1; i++)
    {
        for (int j = 1; j < cols - 1; j++)
        {
            double advection = computeAdvection(i, j);
            double diffusion = computeDiffusion(i, j);
            double decay = computeDecay(i, j);

            // Forward Euler time integration
            temp_grid[i][j] = grid[i][j] + TIME_STEP * (advection + diffusion - decay);

            // Ensure non-negative concentration
            temp_grid[i][j] = std::max(0.0, temp_grid[i][j]);
        }
    }

    // Swap grids
    grid.swap(temp_grid);

    // Apply boundary conditions
    applyBoundaryConditions();
}

int ContaminationSimulation::countUncontaminatedBlocks()
{
    int count = 0;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (grid[i][j] < CONTAMINATION_THRESHOLD)
            {
                count++;
            }
        }
    }
    return count;
}

void ContaminationSimulation::printResults()
{
    std::cout << "\n=== Simulation Results ===" << std::endl;
    std::cout << "Grid Size: " << rows << "x" << cols << std::endl;
    std::cout << "Total Blocks: " << rows * cols << std::endl;
    std::cout << "Uncontaminated Blocks: " << countUncontaminatedBlocks() << std::endl;
    std::cout << "Total Contamination: " << std::fixed << std::setprecision(6)
              << getTotalContamination() << std::endl;

    // Print center region
    std::cout << "\nCenter Region (20x20 around contamination source):" << std::endl;
    int center_i = INITIAL_X;
    int center_j = INITIAL_Y;
    int half_size = 10;

    for (int i = std::max(0, center_i - half_size);
         i < std::min(rows, center_i + half_size); i++)
    {
        for (int j = std::max(0, center_j - half_size);
             j < std::min(cols, center_j + half_size); j++)
        {
            std::cout << std::setw(8) << std::fixed << std::setprecision(2)
                      << grid[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

void ContaminationSimulation::printGrid(int maxRows, int maxCols)
{
    std::cout << "\nGrid (first " << maxRows << "x" << maxCols << "):" << std::endl;
    for (int i = 0; i < std::min(rows, maxRows); i++)
    {
        for (int j = 0; j < std::min(cols, maxCols); j++)
        {
            std::cout << std::setw(8) << std::fixed << std::setprecision(2)
                      << grid[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

double ContaminationSimulation::getTotalContamination()
{
    double total = 0.0;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            total += grid[i][j];
        }
    }
    return total;
}
