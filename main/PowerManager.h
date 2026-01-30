#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <stdbool.h>

// Inicjalizacja (uruchamia taska w tle)
void PowerManager_init();

// Reset licznika bezczynności (wywoływać przy każdym kliknięciu w Keypad)
void PowerManager_reset_timer();

// Czy jesteśmy w trybie "tylko wygaszacz" (Level 1)?
bool PowerManager_is_display_off();

// Wymuszenie głębokiego snu (np. z menu lub przy krytycznej baterii)
void PowerManager_deep_sleep();

#endif  // POWER_MANAGER_H