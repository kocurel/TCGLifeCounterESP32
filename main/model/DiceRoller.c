#include "DiceRoller.h"

#include <esp_random.h>

/* --- Core Logic --- */

/**
 * Generates a random integer between 1 and the specified number of sides.
 * Uses the ESP32 hardware random number generator (HWRNG).
 */
int roll_die(int sides) {
    if (sides <= 0) {
        return 0;
    }

    // esp_random() returns a true hardware-generated 32-bit random value
    return (esp_random() % sides) + 1;
}

/* --- Data Definitions --- */

/**
 * Human-readable names for the supported dice types.
 * Indices correspond to the selection list used in the GUI.
 */
const char* DICE_NAMES[] = {"Coin", "D3",  "D4",  "D6",  "D8",
                            "D10",  "D12", "D20", "D100"};