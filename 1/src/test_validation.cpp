#include "heat_diffusion.h"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <cmath>

// Compare two matrices with a tolerance
bool compare_matrices(float *a, float *b, int size, double tolerance = 1e-4)
{
    double max_diff = 0.0;
    int diff_count = 0;

    for (int i = 0; i < size; i++)
    {
        double diff = std::abs(a[i] - b[i]);
        if (diff > tolerance)
        {
            diff_count++;
            max_diff = std::max(max_diff, diff);
        }
    }

    if (diff_count > 0)
    {
        std::cout << "Differences found: " << diff_count << " elements" << std::endl;
        std::cout << "Max difference: " << max_diff << std::endl;
        return false;
    }

    return true;
}

int main()
{
    std::cout << "=== Heat Diffusion Validation Test ===" << std::endl;
    std::cout << "Matrix size: " << N << "x" << N << std::endl;
    std::cout << "Iterations: " << ITERATIONS << std::endl;
    std::cout << std::endl;

    // Allocate matrices for all three methods
    float *seq_matrix = new float[N * N];
    float *par_matrix = new float[N * N];
    float *tile_matrix = new float[N * N];

    // Initialize all with same data
    init_heat_map(seq_matrix);
    memcpy(par_matrix, seq_matrix, N * N * sizeof(float));
    memcpy(tile_matrix, seq_matrix, N * N * sizeof(float));

    // Run sequential
    std::cout << "Running sequential..." << std::endl;
    float seq_time = sequential_heat_diffusion(seq_matrix, HEAT_KERNEL, ITERATIONS);
    std::cout << "Sequential: " << std::fixed << std::setprecision(3)
              << seq_time << " s" << std::endl;

    // Run parallel
    std::cout << "Running parallel..." << std::endl;
    float par_time = parallel_heat_diffusion(par_matrix, HEAT_KERNEL, ITERATIONS);
    std::cout << "Parallel:   " << std::fixed << std::setprecision(3)
              << par_time << " s" << std::endl;

    // Run tiled
    std::cout << "Running tiled..." << std::endl;
    float tile_time = parallel_heat_diffusion_tiled(tile_matrix, HEAT_KERNEL, ITERATIONS);
    std::cout << "Tiled:      " << std::fixed << std::setprecision(3)
              << tile_time << " s" << std::endl;
    std::cout << std::endl;

    // Validate results
    std::cout << "=== Validation Results ===" << std::endl;

    std::cout << "Sequential vs Parallel: ";
    if (compare_matrices(seq_matrix, par_matrix, N * N))
    {
        std::cout << "✓ PASS" << std::endl;
    }
    else
    {
        std::cout << "✗ FAIL" << std::endl;
        delete[] seq_matrix;
        delete[] par_matrix;
        delete[] tile_matrix;
        return 1;
    }

    std::cout << "Sequential vs Tiled:    ";
    if (compare_matrices(seq_matrix, tile_matrix, N * N))
    {
        std::cout << "✓ PASS" << std::endl;
    }
    else
    {
        std::cout << "✗ FAIL" << std::endl;
        delete[] seq_matrix;
        delete[] par_matrix;
        delete[] tile_matrix;
        return 1;
    }

    std::cout << std::endl;
    std::cout << "All implementations produce identical results!" << std::endl;

    // Cleanup
    delete[] seq_matrix;
    delete[] par_matrix;
    delete[] tile_matrix;

    return 0;
}
