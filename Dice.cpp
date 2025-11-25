#include "Dice.h"
#include <random>
#include <chrono>

// Constructor to seed the random number generator
Dice::Dice() {
    // Seed with a combination of high-resolution clock and random_device
    // This provides a good balance of randomness and uniqueness across runs.
    srand(static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
}

// Rolls a die with a given number of sides
int Dice::roll(int sides) {
    if (sides < 1) {
        return 0; // Invalid number of sides
    }
    // Generate a random number between 1 and 'sides'
    return (rand() % sides) + 1;
}
