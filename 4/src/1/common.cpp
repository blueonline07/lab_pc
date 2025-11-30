#include "common.h"

bool read_file(double **grid, char *filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file " << filename << std::endl;
        return 1;
    }
    std::string line;
    for (int i = 1; i <= N; i++)
    {
        if (!std::getline(file, line))
        {
            return false;
        }
        std::istringstream ss(line);
        for (int j = 1; j <= N; j++)
        {
            std::string token;
            if (!std::getline(ss, token, ','))
            {
                return 1;
            }
            grid[i][j] = std::stod(token);
        }
    }

    file.close();
    return true;
}