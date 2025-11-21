#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>

// Physical constants
constexpr int GRID_SIZE = 4000; // 4000x4000 grid
constexpr double DX = 10.0;     // 100 m² per cell
constexpr double DY = 10.0;
constexpr int SIMULATION_TIME = 100; // 100 steps (100 seconds with dt=1.0)
constexpr double TIME_STEP = 1.0;    // 1.0 second per step

// Physical coefficients
constexpr double DIFFUSION_COEFF = 1.0;  // D (stabilized)
constexpr double DECAY_RATE = 3e-5;      // λ
constexpr double DEPOSITION_RATE = 1e-4; // k
constexpr double WIND_X = 0.33;          // ux (stabilized)
constexpr double WIND_Y = 0.14;          // uy (stabilized)

// Initial conditions
constexpr double INITIAL_CONTAMINATION = 1000.0;
constexpr int INITIAL_X = GRID_SIZE / 2; // Centered initial release
constexpr int INITIAL_Y = GRID_SIZE / 2;

// Threshold for uncontaminated blocks
constexpr double CONTAMINATION_THRESHOLD = 1e-6;

class ContaminationSimulation
{
private:
    std::vector<std::vector<double>> grid;
    std::vector<std::vector<double>> temp_grid;
    int rows, cols;

    // Helper functions
    void applyBoundaryConditions();
    double computeAdvection(int i, int j);
    double computeDiffusion(int i, int j);
    double computeDecay(int i, int j);

public:
    ContaminationSimulation(int rows, int cols);
    ~ContaminationSimulation() = default;

    // Core simulation functions
    void initialize();
    void simulateStep();
    void saveToCSV(const char* filename) const;
    bool loadFromCSV(const char* filename);

    // Accessors for MPI implementation
    std::vector<std::vector<double>> &getGrid() { return grid; }
    const std::vector<std::vector<double>> &getGrid() const { return grid; }
    int getRows() const { return rows; }
    int getCols() const { return cols; }
};

#endif // SIMULATION_H
