#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include <chrono>
#include <iostream>

// Utility functions for simulation
namespace SimulationUtils {
    
    // Time measurement utilities
    class Timer {
    private:
        std::chrono::high_resolution_clock::time_point start_time;
        std::string timer_name;
        
    public:
        Timer(const std::string& name) : timer_name(name) {
            start_time = std::chrono::high_resolution_clock::now();
        }
        
        ~Timer() {
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            std::cout << timer_name << " took " << duration.count() << " ms" << std::endl;
        }
        
        double elapsed() {
            auto current_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
            return duration.count();
        }
    };
    
    // Grid utilities
    void printGridSubset(const std::vector<std::vector<double>>& grid, 
                        int startRow, int endRow, int startCol, int endCol,
                        const std::string& title = "");
    
    double calculateTotalMass(const std::vector<std::vector<double>>& grid);
    
    // Validation functions
    bool validateGrid(const std::vector<std::vector<double>>& grid);
    bool checkBoundaryConditions(const std::vector<std::vector<double>>& grid);
    
    // Performance utilities
    void printPerformanceStats(double sequential_time, double parallel_time, int num_processes);
    
    // File I/O utilities
    void saveGridToFile(const std::vector<std::vector<double>>& grid, const std::string& filename);
    void loadGridFromFile(std::vector<std::vector<double>>& grid, const std::string& filename);
}

#endif // UTILS_H
