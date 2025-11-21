#include "heat_diffusion.h"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <string>

int main(int argc, char* argv[])
{
    // Parse command line arguments
    bool save_output = true;
    const char* input_file = nullptr;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--no-output" || arg == "--skip-csv") {
            save_output = false;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [options] [input_csv]\n"
                      << "Options:\n"
                      << "  --no-output, --skip-csv    Skip CSV output generation\n"
                      << "  --help, -h                 Show this help message\n"
                      << "Arguments:\n"
                      << "  input_csv                  Load initial conditions from CSV file\n";
            return 0;
        } else if (arg[0] != '-') {
            input_file = argv[i];
        }
    }

    // Allocate matrices for sequential and parallel runs
    float *seq_matrix = new float[N * N];
    float *par_matrix = new float[N * N];
    float *tile_matrix = new float[N * N];

    // Initialize heat map - from CSV if provided, otherwise generate default
    if (input_file) {
        // Load from CSV file provided as command line argument
        if (!load_matrix_from_csv(seq_matrix, input_file)) {
            std::cerr << "Failed to load input CSV. Using default initialization." << std::endl;
            init_heat_map(seq_matrix);
        }
    } else {
        // Use default initialization
        init_heat_map(seq_matrix);
    }
    
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

    // Save results to CSV files (if enabled)
    if (save_output) {
        save_matrix_to_csv(seq_matrix, "lab1_sequential_result.csv");
        save_matrix_to_csv(par_matrix, "lab1_parallel_result.csv");
        save_matrix_to_csv(tile_matrix, "lab1_tiled_result.csv");
    } else {
        std::cout << "CSV output skipped.\n";
    }

    // Cleanup
    delete[] seq_matrix;
    delete[] par_matrix;
    delete[] tile_matrix;

    return 0;
}
