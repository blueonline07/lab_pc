#include <iomanip>
#include <iostream>
#include "physics_utils.h"

int main()
{
    double yield_kt = 5000.0;   // kilotons
    double distance_m = 1000.0; // 1 km

    double overpressure = PhysicsUtils::calculatePeakOverpressure(distance_m, yield_kt);
    double arrival_time = PhysicsUtils::calculateArrivalTime(distance_m);
    bool reached_at_1s = PhysicsUtils::hasShockReached(distance_m, 1);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Overpressure(1km): " << overpressure << " kPa\n";
    std::cout << "ArrivalTime(1km): " << arrival_time << " s\n";
    std::cout << "Reached@1s: " << (reached_at_1s ? "yes" : "no") << "\n";
    return 0;
}

