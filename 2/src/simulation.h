#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>

constexpr int GRID_SIZE = 4000;
constexpr double DX = 10.0;
constexpr double DY = 10.0;
constexpr int SIMULATION_TIME = 100;
constexpr double TIME_STEP = 1.0;

constexpr double DIFFUSION_COEFF = 1.0;
constexpr double DECAY_RATE = 3e-5;
constexpr double DEPOSITION_RATE = 1e-4;
constexpr double WIND_X = 0.33;
constexpr double WIND_Y = 0.14;

constexpr double INITIAL_CONTAMINATION = 1000.0;
constexpr int INITIAL_X = GRID_SIZE / 2;
constexpr int INITIAL_Y = GRID_SIZE / 2;

constexpr double CONTAMINATION_THRESHOLD = 1e-6;

class ContaminationSimulation
{
private:
    std::vector<std::vector<double>> grid;
    std::vector<std::vector<double>> temp_grid;
    int rows, cols;

    void applyBoundaryConditions();
    double computeAdvection(int i, int j);
    double computeDiffusion(int i, int j);
    double computeDecay(int i, int j);

public:
    ContaminationSimulation(int rows, int cols);
    ~ContaminationSimulation() = default;

    void initialize();
    void simulateStep();
    void saveToCSV(const char* filename) const;
    bool loadFromCSV(const char* filename);
    void initializeFromFile(const char* filename = nullptr);

    std::vector<std::vector<double>> &getGrid() { return grid; }
    const std::vector<std::vector<double>> &getGrid() const { return grid; }
    int getRows() const { return rows; }
    int getCols() const { return cols; }
};

#endif
