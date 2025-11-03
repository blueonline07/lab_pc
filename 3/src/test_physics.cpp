#include <iostream>
#include <iomanip>
#include "physics_utils.h"

int main() {
    std::cout << "Physics Calculations Verification" << std::endl;
    std::cout << "=================================" << std::endl;
    
    // Test with known values
    double yield_kt = 5000.0; // 5000 kilotons
    double distance_km = 1.0; // 1 km = 1000 meters
    double distance_m = distance_km * 1000.0;
    
    std::cout << "Test parameters:" << std::endl;
    std::cout << "Yield: " << yield_kt << " kilotons" << std::endl;
    std::cout << "Distance: " << distance_km << " km (" << distance_m << " m)" << std::endl;
    std::cout << std::endl;
    
    // Calculate scaled distance
    double Z = distance_m * std::pow(yield_kt * 1000, -1.0/3.0);
    std::cout << "Scaled distance Z: " << std::fixed << std::setprecision(6) << Z << " m/kg^(1/3)" << std::endl;
    
    // Calculate intermediate value
    double u = -0.21436 + 1.35034 * std::log10(Z);
    std::cout << "Intermediate value u: " << std::fixed << std::setprecision(6) << u << std::endl;
    
    // Calculate peak overpressure
    double overpressure = PhysicsUtils::calculatePeakOverpressure(distance_m, yield_kt);
    std::cout << "Peak overpressure: " << std::fixed << std::setprecision(2) << overpressure << " kPa" << std::endl;
    
    // Calculate arrival time
    double arrival_time = PhysicsUtils::calculateArrivalTime(distance_m);
    std::cout << "Arrival time: " << std::fixed << std::setprecision(2) << arrival_time << " seconds" << std::endl;
    
    // Test shock wave reach calculation
    std::cout << std::endl;
    std::cout << "Shock wave reach test:" << std::endl;
    for (int time_step = 0; time_step <= 5; time_step++) {
        bool reached = PhysicsUtils::hasShockReached(distance_m, time_step);
        std::cout << "Time step " << time_step << ": " << (reached ? "REACHED" : "not reached") << std::endl;
    }
    
    return 0;
}
