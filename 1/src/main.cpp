#include "heat_diffusion.h"
#include <iostream>
#include <iomanip>
#include <cstring>

int main()
{

    // Allocate matrices for sequential and parallel runs
    float *seq_matrix = new float[N * N];
    float *par_matrix = new float[N * N];
    float *tile_matrix = new float[N * N];

    // Initialize heat map
    init_heat_map(seq_matrix);
    memcpy(par_matrix, seq_matrix, N * N * sizeof(float));
    memcpy(tile_matrix, seq_matrix, N * N * sizeof(float));

    // Run sequential implementation
    float seq_time = sequential_heat_diffusion(seq_matrix, HEAT_KERNEL, ITERATIONS);

    // Run parallel implementation (baseline collapse(2))
    float par_time = parallel_heat_diffusion(par_matrix, HEAT_KERNEL, ITERATIONS);

    // Run tiled + halo padded parallel implementation
    float tile_time = parallel_heat_diffusion_tiled(tile_matrix, HEAT_KERNEL, ITERATIONS);
    // Minimal output: print only times
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Sequential: " << seq_time << " s\n";
    std::cout << "Parallel:  " << par_time << " s\n";
    std::cout << "Tiled:     " << tile_time << " s\n";

    // Cleanup
    delete[] seq_matrix;
    delete[] par_matrix;
    delete[] tile_matrix;

    return 0;
}
