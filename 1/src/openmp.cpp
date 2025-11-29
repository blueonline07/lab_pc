#include "common.h"
#include <omp.h>

int main(int argc, char *argv[])
{
    double kernel[3][3] = {
        {0.05, 0.1, 0.05},
        {0.1, 0.4, 0.1},
        {0.05, 0.1, 0.05},
    };

    double **grid = new double *[GRID_SIZE + 2];
    double **new_grid = new double *[GRID_SIZE + 2];
    for (int i = 0; i <= GRID_SIZE + 1; i++)
    {
        grid[i] = new double[GRID_SIZE + 2];
        new_grid[i] = new double[GRID_SIZE + 2];
        for (int j = 0; j <= GRID_SIZE + 1; j++)
        {
            grid[i][j] = 30.0;
            new_grid[i][j] = 30.0;
        }
    }
    if (!read_file(grid, argv[1]))
        return 1;

    double t0 = omp_get_wtime();
    for (int t = 0; t < NUM_ITERS; t++)
    {
#pragma omp parallel for collapse(2)

        for (int i = 1; i <= GRID_SIZE; i++)
        {
            for (int j = 1; j <= GRID_SIZE; j++)
            {
                double sum = 0;
                for (int ki = 0; ki < 3; ki++)
                {
                    for (int kj = 0; kj < 3; kj++)
                    {
                        int ni = i + ki - 1;
                        int nj = j + kj - 1;

                        sum += grid[ni][nj] * kernel[ki][kj];
                    }
                }
                new_grid[i][j] = sum;
            }
        }

        double **temp = grid;
        grid = new_grid;
        new_grid = temp;
    }

    std::cout << omp_get_wtime() - t0;

    // for (int i = 0; i <= GRID_SIZE + 1; i++)
    // {
    //     delete[] grid[i];
    //     delete[] new_grid[i];
    // }
    // delete[] grid;
    // delete[] new_grid;
}