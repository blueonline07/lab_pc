#include "common.h"
#include <omp.h>

#define TILE_SIZE 64

int main(int argc, char *argv[])
{
    double kernel[3][3] = {
        {0.05, 0.1, 0.05},
        {0.1, 0.4, 0.1},
        {0.05, 0.1, 0.05},
    };

    double **grid = new double *[N + 2];
    double **new_grid = new double *[N + 2];
    for (int i = 0; i <= N + 1; i++)
    {
        grid[i] = new double[N + 2];
        new_grid[i] = new double[N + 2];
        for (int j = 0; j <= N + 1; j++)
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

#pragma omp parallel for collapse(2) schedule(static)
        for (int ti = 1; ti <= N; ti += TILE_SIZE)
        {
            for (int tj = 1; tj <= N; tj += TILE_SIZE)
            {

                int i_end = std::min(ti + TILE_SIZE, N + 1);
                int j_end = std::min(tj + TILE_SIZE, N + 1);

                for (int i = ti; i < i_end; i++)
                {
                    for (int j = tj; j < j_end; j++)
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
            }
        }

        double **temp = grid;
        grid = new_grid;
        new_grid = temp;
    }
    std::cout << omp_get_wtime() - t0;
    for (int i = 0; i <= N + 1; i++)
    {
        delete[] grid[i];
        delete[] new_grid[i];
    }
    delete[] grid;
    delete[] new_grid;

    return 0;
}