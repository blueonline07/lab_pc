#include "simulation.h"
#include <fstream>
#include <sstream>
#include <iostream>

ContaminationSimulation::ContaminationSimulation(int rows, int cols)
    : rows(rows), cols(cols)
{
    grid.resize(rows, std::vector<double>(cols, 0.0));
    temp_grid.resize(rows, std::vector<double>(cols, 0.0));
}

void ContaminationSimulation::initialize()
{
    if (INITIAL_X < rows && INITIAL_Y < cols)
    {
        grid[INITIAL_X][INITIAL_Y] = INITIAL_CONTAMINATION;
    }

    applyBoundaryConditions();
}

void ContaminationSimulation::applyBoundaryConditions()
{
    for (int i = 0; i < rows; i++)
    {
        grid[i][0] = 0.0;
        grid[i][cols - 1] = 0.0;
    }
    for (int j = 0; j < cols; j++)
    {
        grid[0][j] = 0.0;
        grid[rows - 1][j] = 0.0;
    }
}

double ContaminationSimulation::computeAdvection(int i, int j)
{
    double advection_x = 0.0;
    double advection_y = 0.0;

    if (i > 0)
    {
        advection_x = WIND_X * (grid[i][j] - grid[i - 1][j]) / DX;
    }
    if (j > 0)
    {
        advection_y = WIND_Y * (grid[i][j] - grid[i][j - 1]) / DY;
    }

    return advection_x + advection_y;
}

double ContaminationSimulation::computeDiffusion(int i, int j)
{
    double diffusion = 0.0;

    if (i > 0 && i < rows - 1)
    {
        diffusion += DIFFUSION_COEFF * (grid[i + 1][j] - 2 * grid[i][j] + grid[i - 1][j]) / (DX * DX);
    }
    if (j > 0 && j < cols - 1)
    {
        diffusion += DIFFUSION_COEFF * (grid[i][j + 1] - 2 * grid[i][j] + grid[i][j - 1]) / (DY * DY);
    }

    return diffusion;
}

double ContaminationSimulation::computeDecay(int i, int j)
{
    return (DECAY_RATE + DEPOSITION_RATE) * grid[i][j];
}

void ContaminationSimulation::simulateStep()
{
    for (int i = 1; i < rows - 1; i++)
    {
        for (int j = 1; j < cols - 1; j++)
        {
            double advection = computeAdvection(i, j);
            double diffusion = computeDiffusion(i, j);
            double decay = computeDecay(i, j);

            temp_grid[i][j] = grid[i][j] + TIME_STEP * (-advection + diffusion - decay);
            temp_grid[i][j] = std::max(0.0, temp_grid[i][j]);
        }
    }

    grid.swap(temp_grid);
    applyBoundaryConditions();
}

void ContaminationSimulation::saveToCSV(const char* filename) const
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    }

    std::cout << "Writing results to " << filename << "..." << std::flush;

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            file << grid[i][j];
            if (j < cols - 1) file << ",";
        }
        file << "\n";
    }

    file.close();
    std::cout << " Done!" << std::endl;
}

bool ContaminationSimulation::loadFromCSV(const char* filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file " << filename << " for reading." << std::endl;
        return false;
    }

    std::cout << "Loading initial conditions from " << filename << "..." << std::flush;

    std::string line;
    int row = 0;

    while (std::getline(file, line) && row < rows)
    {
        std::stringstream ss(line);
        std::string value;
        int col = 0;

        while (std::getline(ss, value, ',') && col < cols)
        {
            grid[row][col] = std::stod(value);
            col++;
        }

        if (col != cols)
        {
            std::cerr << "\nError: Row " << row << " has " << col << " columns, expected " << cols << std::endl;
            file.close();
            return false;
        }

        row++;
    }

    file.close();

    if (row != rows)
    {
        std::cerr << "\nError: File has " << row << " rows, expected " << rows << std::endl;
        return false;
    }

    std::cout << " Done!" << std::endl;
    
    applyBoundaryConditions();
    
    return true;
}

void ContaminationSimulation::initializeFromFile(const char* filename) {
    if (filename && loadFromCSV(filename)) {
        return;
    }
    if (filename) {
        std::cerr << "Failed to load input CSV. Using default initialization." << std::endl;
    }
    initialize();
}
