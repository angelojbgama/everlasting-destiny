#ifndef DICE_H
#define DICE_H

class Dice {
public:
    Dice(); // Constructor to seed the random number generator
    int roll(int sides); // Rolls a die with a given number of sides
};

#endif // DICE_H
