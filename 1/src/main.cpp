#include "heat_diffusion.h"
#include <iostream>
#include <iomanip>
#include <cstring>

int main() {
    std::cout << "=== Heat Diffusion Simulation ===" << std::endl;
    std::cout << "Matrix size: " << N << "x" << N << std::endl;
    std::cout << "Iterations: " << ITERATIONS << std::endl;
    std::cout << "Baseline temperature: " << BASELINE_TEMP << "°C" << std::endl;
    std::cout << "OpenMP threads: " << omp_get_max_threads() << std::endl;
    std::cout << std::endl;
    
    // Allocate matrices for sequential and parallel runs
    float* seq_matrix = new float[N * N];
    float* par_matrix = new float[N * N];
    float* tile_matrix = new float[N * N];
    
    // Initialize heat map (nuclear explosion epicenter)
    std::cout << "Initializing heat map..." << std::endl;
    init_heat_map(seq_matrix);
    memcpy(par_matrix, seq_matrix, N * N * sizeof(float));
    memcpy(tile_matrix, seq_matrix, N * N * sizeof(float));
    
    print_matrix_stats(seq_matrix, N * N, "Initial");
    std::cout << std::endl;
    
    // Run sequential implementation
    std::cout << "Running sequential implementation..." << std::endl;
    float seq_time = sequential_heat_diffusion(seq_matrix, HEAT_KERNEL, ITERATIONS);
    std::cout << "Sequential time: " << std::fixed << std::setprecision(3) 
              << seq_time << " seconds" << std::endl;
    print_matrix_stats(seq_matrix, N * N, "Sequential Final");
    std::cout << std::endl;
    
    // Run parallel implementation (baseline collapse(2))
    std::cout << "Running parallel implementation..." << std::endl;
    float par_time = parallel_heat_diffusion(par_matrix, HEAT_KERNEL, ITERATIONS);
    std::cout << "Parallel time: " << std::fixed << std::setprecision(3) 
              << par_time << " seconds" << std::endl;
    print_matrix_stats(par_matrix, N * N, "Parallel Final");
    std::cout << std::endl;

    // Run tiled + halo padded parallel implementation
    std::cout << "Running tiled parallel implementation..." << std::endl;
    float tile_time = parallel_heat_diffusion_tiled(tile_matrix, HEAT_KERNEL, ITERATIONS);
    std::cout << "Tiled parallel time: " << std::fixed << std::setprecision(3)
              << tile_time << " seconds" << std::endl;
    print_matrix_stats(tile_matrix, N * N, "Tiled Parallel Final");
    std::cout << std::endl;
    
    // Calculate and display performance metrics
    float speedup = seq_time / par_time;
    float efficiency = speedup / omp_get_max_threads();
    float speedup_tiled = seq_time / tile_time;
    float efficiency_tiled = speedup_tiled / omp_get_max_threads();
    
    std::cout << "=== Performance Results ===" << std::endl;
    std::cout << "Sequential time: " << seq_time << " seconds" << std::endl;
    std::cout << "Parallel time:   " << par_time << " seconds" << std::endl;
    std::cout << "Tiled time:      " << tile_time << " seconds" << std::endl;
    std::cout << "Speedup:         " << std::fixed << std::setprecision(2) 
              << speedup << "x" << std::endl;
    std::cout << "Efficiency:      " << std::fixed << std::setprecision(1) 
              << efficiency * 100 << "%" << std::endl;
    std::cout << "Speedup (tiled): " << std::fixed << std::setprecision(2)
              << speedup_tiled << "x" << std::endl;
    std::cout << "Efficiency (tiled): " << std::fixed << std::setprecision(1)
              << efficiency_tiled * 100 << "%" << std::endl;
    std::cout << std::endl;
    
    // Validate correctness
    std::cout << "=== Validation ===" << std::endl;
    bool is_correct = validate_results(seq_matrix, par_matrix, N * N);
    bool is_correct_tiled = validate_results(seq_matrix, tile_matrix, N * N);
    
    if(is_correct) {
        std::cout << "✓ Sequential and parallel implementations produce identical results!" << std::endl;
    } else {
        std::cout << "✗ Sequential and parallel implementations differ!" << std::endl;
        return 1;
    }
    if(is_correct_tiled) {
        std::cout << "✓ Sequential and tiled-parallel implementations produce identical results!" << std::endl;
    } else {
        std::cout << "✗ Sequential and tiled-parallel implementations differ!" << std::endl;
        return 1;
    }
    
    // Cleanup
    delete[] seq_matrix;
    delete[] par_matrix;
    delete[] tile_matrix;
    
    std::cout << std::endl << "Simulation completed successfully!" << std::endl;
    return 0;
}
