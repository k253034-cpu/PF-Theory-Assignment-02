#include <stdio.h>

// Recursive function to track fuel across planets
int calculateFuel(int fuel, int consumption, int recharge, int solarBonus,
                  int planet, int totalPlanets) {

    // ---------- BASE CASE ----------
    if (fuel <= 0) {
        printf("Mission Failed! Fuel exhausted before reaching Planet %d.\n", planet);
        return 0;
    }

    if (planet > totalPlanets) {
        printf("Mission Successful! Spacecraft completed the journey.\n");
        return 1;
    }

    // ---------- RECURSIVE CASE ----------
    printf("\n--- Arriving at Planet %d ---\n", planet);

    // 1. Fuel consumed at every planet
    fuel -= consumption;

    // Check if fuel is already gone
    if (fuel <= 0) {
        printf("Planet %d: Fuel Remaining = 0 (exhausted)\n", planet);
        return calculateFuel(0, consumption, recharge, solarBonus,
                             planet + 1, totalPlanets);
    }

    // 2. Gravitational assist ? adds recharge fuel
    fuel += recharge;

    // 3. Solar Recharge every 4th planet
    if (planet % 4 == 0) {
        printf("Solar Recharge Activated! +%d Fuel\n", solarBonus);
        fuel += solarBonus;
    }

    // Print updated fuel
    printf("Planet %d: Fuel Remaining = %d\n", planet, fuel);

    // Move to next planet recursively
    return calculateFuel(fuel, consumption, recharge, solarBonus,
                         planet + 1, totalPlanets);
}

int main() {

    int startingFuel = 500;
    int consumption = 60;
    int recharge = 20;     // gravitational recharge
    int solarBonus = 50;   // extra bonus after every 4th planet
    int totalPlanets = 10;

    printf("=== Spacecraft Fuel Simulation ===\n");

    calculateFuel(startingFuel, consumption, recharge, solarBonus, 1, totalPlanets);

    return 0;
}
