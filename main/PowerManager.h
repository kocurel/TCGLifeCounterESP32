#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <stdbool.h>

// Time Configuration
#define POWER_TIMEOUT_DISPLAY_MS (2 * 60 * 1000)  // 2 minuty -> Wyłącz ekran
#define POWER_TIMEOUT_DEEP_SLEEP_MS (15 * 60 * 1000)  // 15 minut -> Zabij ESP

// Inicjalizacja
void PowerManager_init();

// Reset licznika (wywoływać przy każdym inputcie)
void PowerManager_reset_timer();

// Czy jesteśmy w trybie "tylko wygaszacz"?
bool PowerManager_is_display_off();

void PowerManager_deep_sleep();

#endif  // POWER_MANAGER_H