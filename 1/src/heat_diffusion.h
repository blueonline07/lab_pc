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
void init_heat_map(float *matrix);
float sequential_heat_diffusion(float *input, const float kernel[3][3], int iterations);
float parallel_heat_diffusion(float *input, const float kernel[3][3], int iterations);
float parallel_heat_diffusion_tiled(float *input, const float kernel[3][3], int iterations);
void save_matrix_to_csv(float *matrix, const char *filename);
bool load_matrix_from_csv(float *matrix, const char *filename);

#endif // HEAT_DIFFUSION_H
