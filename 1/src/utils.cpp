#include "heat_diffusion.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstring>
#include <algorithm>

void init_heat_map(float* matrix) {
    int center = N / 2;
    float max_temp = 1000.0f;  // Peak temperature at epicenter
    
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            // Calculate distance from center
            float dx = i - center;
            float dy = j - center;
            float dist = sqrt(dx * dx + dy * dy);
            
            // Gaussian heat distribution: peak at center, decreasing outward
            float sigma = N * 0.1f;  // Controls spread of heat
            float temp = BASELINE_TEMP + max_temp * exp(-(dist * dist) / (2 * sigma * sigma));
            
            matrix[i * N + j] = temp;
        }
    }
}

bool validate_results(float* seq, float* par, int size) {
    const float epsilon = 1e-4f;
    float max_diff = 0.0f;
    int diff_count = 0;
    
    for(int i = 0; i < size; i++) {
        float diff = fabs(seq[i] - par[i]);
        if(diff > epsilon) {
            diff_count++;
            max_diff = std::max(max_diff, diff);
        }
    }
    
    std::cout << "Validation Results:" << std::endl;
    std::cout << "  Max difference: " << max_diff << std::endl;
    std::cout << "  Values exceeding tolerance: " << diff_count << " / " << size << std::endl;
    std::cout << "  Validation: " << (diff_count == 0 ? "PASSED" : "FAILED") << std::endl;
    
    return diff_count == 0;
}

void print_matrix_stats(float* matrix, int size, const char* name) {
    float min_val = matrix[0];
    float max_val = matrix[0];
    float sum = 0.0f;
    
    for(int i = 0; i < size; i++) {
        min_val = std::min(min_val, matrix[i]);
        max_val = std::max(max_val, matrix[i]);
        sum += matrix[i];
    }
    
    float avg = sum / size;
    
    std::cout << name << " Statistics:" << std::endl;
    std::cout << "  Min: " << min_val << std::endl;
    std::cout << "  Max: " << max_val << std::endl;
    std::cout << "  Avg: " << avg << std::endl;
}

void save_matrix_to_csv(float* matrix, const char* filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    }
    
    std::cout << "Writing results to " << filename << "..." << std::flush;
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            file << matrix[i * N + j];
            if (j < N - 1) file << ",";
        }
        file << "\n";
    }
    
    file.close();
    std::cout << " Done!" << std::endl;
}

bool load_matrix_from_csv(float* matrix, const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for reading." << std::endl;
        return false;
    }
    
    std::cout << "Loading initial conditions from " << filename << "..." << std::flush;
    
    std::string line;
    int row = 0;
    
    while (std::getline(file, line) && row < N) {
        std::stringstream ss(line);
        std::string value;
        int col = 0;
        
        while (std::getline(ss, value, ',') && col < N) {
            matrix[row * N + col] = std::stof(value);
            col++;
        }
        
        if (col != N) {
            std::cerr << "\nError: Row " << row << " has " << col << " columns, expected " << N << std::endl;
            file.close();
            return false;
        }
        
        row++;
    }
    
    file.close();
    
    if (row != N) {
        std::cerr << "\nError: File has " << row << " rows, expected " << N << std::endl;
        return false;
    }
    
    std::cout << " Done!" << std::endl;
    return true;
}
