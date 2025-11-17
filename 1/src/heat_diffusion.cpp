#include "heat_diffusion.h"
#include <cstring>
#include <algorithm>

// Heat diffusion kernel as specified in lab requirements
const float HEAT_KERNEL[3][3] = {
    {0.05f, 0.1f, 0.05f},
    {0.1f, 0.4f, 0.1f},
    {0.05f, 0.1f, 0.05f}};

// Unified naive implementation: runs sequentially or in parallel based on flag
static float heat_diffusion_naive(float *input, const float kernel[3][3], int iterations, bool do_parallel)
{
    double start_time = omp_get_wtime();

    float *buffer1 = new float[N * N];
    float *buffer2 = new float[N * N];

    memcpy(buffer1, input, N * N * sizeof(float));

    float *current_input = buffer1;
    float *current_output = buffer2;

    for (int iter = 0; iter < iterations; iter++)
    {
// Perform convolution (parallelized on demand)
#pragma omp parallel for collapse(2) schedule(static) if (do_parallel)
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                float sum = 0.0f;

                for (int ki = 0; ki < KERNEL_SIZE; ki++)
                {
                    for (int kj = 0; kj < KERNEL_SIZE; kj++)
                    {
                        int in_i = i + ki - PADDING;
                        int in_j = j + kj - PADDING;

                        if (in_i >= 0 && in_i < N && in_j >= 0 && in_j < N)
                        {
                            sum += current_input[in_i * N + in_j] * kernel[ki][kj];
                        }
                        else
                        {
                            sum += BASELINE_TEMP * kernel[ki][kj];
                        }
                    }
                }

                current_output[i * N + j] = sum;
            }
        }

        std::swap(current_input, current_output);
    }

    memcpy(input, current_input, N * N * sizeof(float));

    delete[] buffer1;
    delete[] buffer2;

    double end_time = omp_get_wtime();
    return (float)(end_time - start_time);
}

float sequential_heat_diffusion(float *input, const float kernel[3][3], int iterations)
{
    return heat_diffusion_naive(input, kernel, iterations, false);
}

float parallel_heat_diffusion(float *input, const float kernel[3][3], int iterations)
{
    return heat_diffusion_naive(input, kernel, iterations, true);
}

float parallel_heat_diffusion_tiled(float *input, const float kernel[3][3], int iterations)
{
    const int stride = N + 2; // halo padding on all sides
    const int tile = 64;      // default tile size; can be tuned

    double start_time = omp_get_wtime();

    // Allocate halo-padded ping-pong buffers
    float *pad_a = new float[stride * stride];
    float *pad_b = new float[stride * stride];

    auto set_halo = [&](float *buf)
    {
        // Top and bottom rows
        for (int j = 0; j < stride; j++)
        {
            buf[0 * stride + j] = BASELINE_TEMP;
            buf[(stride - 1) * stride + j] = BASELINE_TEMP;
        }
        // Left and right columns
        for (int i = 0; i < stride; i++)
        {
            buf[i * stride + 0] = BASELINE_TEMP;
            buf[i * stride + (stride - 1)] = BASELINE_TEMP;
        }
    };

    // Initialize pad_a from input with halo
    set_halo(pad_a);
    for (int i = 0; i < N; i++)
    {
        float *dst_row = &pad_a[(i + 1) * stride + 1];
        const float *src_row = &input[i * N];
        memcpy(dst_row, src_row, N * sizeof(float));
    }

    float *cur_in = pad_a;
    float *cur_out = pad_b;

    for (int iter = 0; iter < iterations; iter++)
    {
        // Maintain halo baseline on output before compute
        set_halo(cur_out);

// Tile over the inner N x N domain
#pragma omp parallel for collapse(2) schedule(static)
        for (int i0 = 0; i0 < N; i0 += tile)
        {
            for (int j0 = 0; j0 < N; j0 += tile)
            {
                const int i_max = std::min(i0 + tile, N);
                const int j_max = std::min(j0 + tile, N);

                for (int i = i0; i < i_max; i++)
                {
                    for (int j = j0; j < j_max; j++)
                    {
                        const int ii = i + 1; // offset due to halo
                        const int jj = j + 1;

                        // Unrolled 3x3 convolution; no boundary checks
                        float sum = 0.0f;
                        // Row -1
                        sum += cur_in[(ii - 1) * stride + (jj - 1)] * kernel[0][0];
                        sum += cur_in[(ii - 1) * stride + (jj)] * kernel[0][1];
                        sum += cur_in[(ii - 1) * stride + (jj + 1)] * kernel[0][2];
                        // Row 0
                        sum += cur_in[(ii)*stride + (jj - 1)] * kernel[1][0];
                        sum += cur_in[(ii)*stride + (jj)] * kernel[1][1];
                        sum += cur_in[(ii)*stride + (jj + 1)] * kernel[1][2];
                        // Row +1
                        sum += cur_in[(ii + 1) * stride + (jj - 1)] * kernel[2][0];
                        sum += cur_in[(ii + 1) * stride + (jj)] * kernel[2][1];
                        sum += cur_in[(ii + 1) * stride + (jj + 1)] * kernel[2][2];

                        cur_out[ii * stride + jj] = sum;
                    }
                }
            }
        }

        // Swap buffers
        std::swap(cur_in, cur_out);
    }

    // Copy inner region back to input
    for (int i = 0; i < N; i++)
    {
        const float *src_row = &cur_in[(i + 1) * stride + 1];
        float *dst_row = &input[i * N];
        memcpy(dst_row, src_row, N * sizeof(float));
    }

    delete[] pad_a;
    delete[] pad_b;

    double end_time = omp_get_wtime();
    return (float)(end_time - start_time);
}
