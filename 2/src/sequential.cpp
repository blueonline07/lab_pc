#include "simulation.h"
#include <chrono>

int main(int argc, char *argv[])
{
    double **grid = new double *[GRID_SIZE + 2];
    double **new_grid = new double *[GRID_SIZE + 2];
    for (int i = 0; i <= GRID_SIZE + 1; i++)
    {
        grid[i] = new double[GRID_SIZE + 2];
        new_grid[i] = new double[GRID_SIZE + 2];
        for (int j = 0; j <= GRID_SIZE + 1; j++)
        {
            grid[i][j] = 0.0;
            new_grid[i][j] = 0.0;
        }
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
            grid[i][j] = std::stod(token);
        }
    }

    file.close();
    auto start = std::chrono::high_resolution_clock::now();
    for (int t = 0; t < SIMULATION_STEPS; t++)
    {
        int total_uncontaminated = 0;

        for (int i = 1; i <= GRID_SIZE; i++)
        {
            for (int j = 1; j <= GRID_SIZE; j++)
            {
                double advection = WIND_X * (grid[i][j] - grid[i - 1][j]) / DX + WIND_Y * (grid[i][j] - grid[i][j - 1]) / DY;
                double diffusion = DIFFUSION_COEFF * (grid[i + 1][j] - 2 * grid[i][j] + grid[i - 1][j]) / (DX * DX) + DIFFUSION_COEFF * (grid[i][j + 1] - 2 * grid[i][j] + grid[i][j - 1]) / (DY * DY);
                double decay = DECAY_RATE * grid[i][j] + DEPOSITION_RATE * grid[i][j];
                new_grid[i][j] = grid[i][j] + TIME_STEP * (-advection + diffusion - decay);
                new_grid[i][j] = std::max(0.0, new_grid[i][j]);

                if (new_grid[i][j] == 0)
                    total_uncontaminated++;
            }
        }
        std::swap(grid, new_grid);
        std::cout << total_uncontaminated << std::endl;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential: " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

    for (int i = 0; i <= GRID_SIZE + 1; i++)
    {
        delete[] grid[i];
    }
    delete[] grid;

    return 0;
}