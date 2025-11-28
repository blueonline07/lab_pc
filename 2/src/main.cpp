#include "simulation.h"
int main(int , char* argv[]){
    double **grid = new double*[GRID_SIZE + 2];
    for (int i = 0; i <= GRID_SIZE + 1; i++) {
        grid[i] = new double[GRID_SIZE + 2];
        for (int j = 0; j <= GRID_SIZE + 1; j++) {
            grid[i][j] = 0.0;
        }
    }
    
    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Failed to open file " << argv[1] << std::endl;
        return 1;
    }
    std::string line;
    for (int i = 1; i <= GRID_SIZE; i++) {
        if (!std::getline(file, line)) {
            return 1;
        }
        std::istringstream ss(line);
        for (int j = 1; j <= GRID_SIZE; j++) {
            std::string token;
            if (!std::getline(ss, token, ',')) {
                return 1;
            }
            grid[i][j] = std::stod(token);
        }
    }
    if(strcmp(argv[2], "sequential") == 0){
        simulate_sequential(grid);
    } else if(strcmp(argv[2], "parallel") == 0){
        simulate_parallel(grid);
    } else {
        std::cerr<< "Invalid mode" << argv[2]<< std::endl;
    }

    file.close();
    for (int i = 0; i <= GRID_SIZE + 1; i++) {
        delete[] grid[i];
    }
    delete[] grid;
    
    return 0;
}