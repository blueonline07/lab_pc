#include "common.h"
#include <chrono>

int sq(int x)
{
    return x * x;
}

int main(int argc, char *argv[])
{
    const double c[9] = {2.611369, -1.690128, 0.00805, 0.336743, -0.005162, -0.080923, -0.004785, 0.007930, 0.000768};
    double **grid = new double *[N];
    for (int i = 0; i < N; i++)
    {
        grid[i] = new double[N];
    }
    auto start = std::chrono::high_resolution_clock::now();
    for (int t = 0; t < TIME; t++)
    {
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                double R = sqrt(sq(i - CENTER_X) + sq(j - CENTER_Y)) * CELL_SIZE;

                if (t >= R / 343.0)
                {
                    double Z = R * pow(W, -1 / 3);
                    double U = -0.21436 + 1.35034 * log10(Z);
                    double log10P = 0.0;
                    for (int i = 0; i < 9; i++)
                    {
                        log10P += c[i] * pow(U, i);
                    }
                    grid[i][j] = pow(log10P, 10);
                }
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    for (int i = 0; i < N; i++)
    {
        delete[] grid[i];
    }
    return 0;
}