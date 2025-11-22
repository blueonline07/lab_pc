#include "heat_diffusion.h"
#include "arg_parser.h"
#include <iostream>
#include <iomanip>
#include <cstring>

int main(int argc, char* argv[])
{
    ProgramOptions opts = parse_arguments(argc, argv, argv[0]);

    float *seq_matrix = new float[N * N];
    float *par_matrix = new float[N * N];
    float *tile_matrix = new float[N * N];

    initialize_matrix(seq_matrix, opts.input_file);
    memcpy(par_matrix, seq_matrix, N * N * sizeof(float));
    memcpy(tile_matrix, seq_matrix, N * N * sizeof(float));

    float seq_time = sequential_heat_diffusion(seq_matrix, HEAT_KERNEL, ITERATIONS);
    float par_time = parallel_heat_diffusion(par_matrix, HEAT_KERNEL, ITERATIONS);
    float tile_time = parallel_heat_diffusion_tiled(tile_matrix, HEAT_KERNEL, ITERATIONS);
    
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Sequential: " << seq_time << " s\n";
    std::cout << "Parallel:  " << par_time << " s\n";
    std::cout << "Tiled:     " << tile_time << " s\n";

    if (opts.save_output) {
        save_matrix_to_csv(seq_matrix, "lab1_sequential_result.csv");
        save_matrix_to_csv(par_matrix, "lab1_parallel_result.csv");
        save_matrix_to_csv(tile_matrix, "lab1_tiled_result.csv");
    } else {
        std::cout << "CSV output skipped.\n";
    }

    delete[] seq_matrix;
    delete[] par_matrix;
    delete[] tile_matrix;

    return 0;
}
