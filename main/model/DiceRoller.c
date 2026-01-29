#include "DiceRoller.h"

#include <stdlib.h>
int roll_die(int sides) {
    if (sides <= 0) {
        return 0;
    }
    return rand() % sides + 1;
}

const char* DICE_NAMES[] = {"Coin", "D3",  "D4",  "D6",  "D8",
                            "D10",  "D12", "D20", "D100"};