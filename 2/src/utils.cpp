#include "utils.h"
#include <fstream>
#include <iomanip>
#include <cmath>

namespace SimulationUtils {
    
    void printGridSubset(const std::vector<std::vector<double>>& grid, 
                        int startRow, int endRow, int startCol, int endCol,
                        const std::string& title) {
        if (!title.empty()) {
            std::cout << title << std::endl;
        }
        
        for (int i = startRow; i < endRow; i++) {
            for (int j = startCol; j < endCol; j++) {
                std::cout << std::setw(8) << std::fixed << std::setprecision(2) 
                          << grid[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }
    
    double calculateTotalMass(const std::vector<std::vector<double>>& grid) {
        double total = 0.0;
        for (const auto& row : grid) {
            for (double value : row) {
                total += value;
            }
        }
        return total;
    }
    
    bool validateGrid(const std::vector<std::vector<double>>& grid) {
        // Check for negative values
        for (const auto& row : grid) {
            for (double value : row) {
                if (value < 0.0) {
                    std::cerr << "Error: Negative concentration detected!" << std::endl;
                    return false;
                }
            }
        }
        
        // Check for NaN or infinite values
        for (const auto& row : grid) {
            for (double value : row) {
                if (std::isnan(value) || std::isinf(value)) {
                    std::cerr << "Error: Invalid concentration value detected!" << std::endl;
                    return false;
                }
            }
        }
        
        return true;
    }
    
    bool checkBoundaryConditions(const std::vector<std::vector<double>>& grid) {
        int rows = grid.size();
        int cols = grid[0].size();
        
        // Check top and bottom boundaries
        for (int j = 0; j < cols; j++) {
            if (grid[0][j] != 0.0 || grid[rows-1][j] != 0.0) {
                std::cerr << "Error: Boundary condition violation at row boundaries!" << std::endl;
                return false;
            }
        }
        
        // Check left and right boundaries
        for (int i = 0; i < rows; i++) {
            if (grid[i][0] != 0.0 || grid[i][cols-1] != 0.0) {
                std::cerr << "Error: Boundary condition violation at column boundaries!" << std::endl;
                return false;
            }
        }
        
        return true;
    }
    
    void printPerformanceStats(double sequential_time, double parallel_time, int num_processes) {
        double speedup = sequential_time / parallel_time;
        double efficiency = speedup / num_processes;
        
        std::cout << "\n=== Performance Statistics ===" << std::endl;
        std::cout << "Sequential Time: " << std::fixed << std::setprecision(3) 
                  << sequential_time << " seconds" << std::endl;
        std::cout << "Parallel Time (" << num_processes << " processes): " 
                  << std::fixed << std::setprecision(3) 
                  << parallel_time << " seconds" << std::endl;
        std::cout << "Speedup: " << std::fixed << std::setprecision(2) 
                  << speedup << "x" << std::endl;
        std::cout << "Efficiency: " << std::fixed << std::setprecision(2) 
                  << efficiency * 100 << "%" << std::endl;
    }
    
    void saveGridToFile(const std::vector<std::vector<double>>& grid, const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << " for writing" << std::endl;
            return;
        }
        
        file << grid.size() << " " << grid[0].size() << std::endl;
        for (const auto& row : grid) {
            for (size_t j = 0; j < row.size(); j++) {
                file << std::fixed << std::setprecision(6) << row[j];
                if (j < row.size() - 1) file << " ";
            }
            file << std::endl;
        }
        
        file.close();
        std::cout << "Grid saved to " << filename << std::endl;
    }
    
    void loadGridFromFile(std::vector<std::vector<double>>& grid, const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << " for reading" << std::endl;
            return;
        }
        
        int rows, cols;
        file >> rows >> cols;
        
        grid.resize(rows, std::vector<double>(cols));
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                file >> grid[i][j];
            }
        }
        
        file.close();
        std::cout << "Grid loaded from " << filename << std::endl;
    }
    
} // namespace SimulationUtils
