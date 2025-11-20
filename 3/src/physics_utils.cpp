#include "physics_utils.h"

// Coefficients for peak overpressure calculation (Kingery-Bulmash data)
const double PhysicsUtils::COEFFS[9] = {
    2.611369, -1.690128, 0.00805, 0.336743, -0.005162,
    -0.080923, -0.004785, 0.007930, 0.000768
};

double PhysicsUtils::calculatePeakOverpressure(double distance, double yield_kt) {
    // Convert distance to meters (already in meters from cell_size multiplication)
    double R = distance;
    
    // Calculate scaled distance Z = R * W^(-1/3)
    // 1 kiloton = 1000 tons = 1,000,000 kg
    double Z = R * std::pow(yield_kt * 1000000.0, -1.0/3.0);
    
    // Calculate u = -0.21436 + 1.35034 * log10(Z)
    double u = -0.21436 + 1.35034 * std::log10(Z);
    
    // Calculate log10(Pso) using polynomial coefficients
    double log10_Pso = 0.0;
    double u_power = 1.0;
    for (int i = 0; i < 9; i++) {
        log10_Pso += COEFFS[i] * u_power;
        u_power *= u;
    }
    
    // Convert back to pressure in kPa
    return std::pow(10.0, log10_Pso);
}

double PhysicsUtils::calculateArrivalTime(double distance) {
    // Simplified: shock wave travels at speed of sound (343 m/s)
    return distance / 343.0;
}

bool PhysicsUtils::hasShockReached(double distance, int time_step) {
    double arrival_time = calculateArrivalTime(distance);
    return time_step >= arrival_time;
}

double PhysicsUtils::calculateDistance(int row, int col, int map_size, double cell_size) {
    double center_row = map_size / 2.0;
    double center_col = map_size / 2.0;
    return std::sqrt(std::pow(row - center_row, 2) + 
                     std::pow(col - center_col, 2)) * cell_size;
}
