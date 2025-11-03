#ifndef PHYSICS_UTILS_H
#define PHYSICS_UTILS_H

#include <cmath>

class PhysicsUtils {
public:
    // Coefficients for Kingery-Bulmash peak overpressure calculation
    static const double COEFFS[9];
    
    // Calculate peak overpressure using Kingery-Bulmash scaling law
    static double calculatePeakOverpressure(double distance, double yield_kt);
    
    // Calculate shock wave arrival time (simplified: speed of sound)
    static double calculateArrivalTime(double distance);
    
    // Check if shock wave has reached a position at given time
    static bool hasShockReached(double distance, int time_step);
    
    // Calculate distance from center to a grid position
    static double calculateDistance(int row, int col, int map_size, double cell_size);
};

#endif // PHYSICS_UTILS_H
