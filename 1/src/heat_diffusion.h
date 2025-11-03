#ifndef HEAT_DIFFUSION_H
#define HEAT_DIFFUSION_H

#include <omp.h>

// Constants
#define N 4000
#define KERNEL_SIZE 3
#define ITERATIONS 100
#define BASELINE_TEMP 30.0f
#define PADDING 1

// Heat diffusion kernel as specified in lab requirements
extern const float HEAT_KERNEL[3][3];

// Function declarations
void init_heat_map(float* matrix);
float sequential_heat_diffusion(float* input, const float kernel[3][3], int iterations);
float parallel_heat_diffusion(float* input, const float kernel[3][3], int iterations);
float parallel_heat_diffusion_tiled(float* input, const float kernel[3][3], int iterations);
bool validate_results(float* seq, float* par, int size);
void print_matrix_stats(float* matrix, int size, const char* name);

#endif // HEAT_DIFFUSION_H
