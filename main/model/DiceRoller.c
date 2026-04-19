#include "DiceRoller.h"

#include <esp_random.h>

/**
 * Generates a random integer between 1 and the specified number of sides.
 * Utilizes the ESP32 hardware random number generator (HWRNG) for entropy.
 */
int roll_die(int sides) {
    if (sides <= 0) {
        return 0;
    }

    /* Standard modulo distribution for dice face calculation */
    return (esp_random() % sides) + 1;
}

/**
 * Supported dice labels for GUI selection and rendering.
 * Mapping corresponds to the internal sides_map used in input handling.
 */
const char* DICE_NAMES[] = {"Coin", "D3",  "D4",  "D6",  "D8",
                            "D10",  "D12", "D20", "D100"};