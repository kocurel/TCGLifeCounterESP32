#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <stdbool.h>

/**
 * Initializes the power management system and spawns the monitoring task.
 */
void PowerManager_init(void);

/**
 * Resets the inactivity timers.
 * Should be called on every user interaction (e.g., keypad events).
 */
void PowerManager_reset_timer(void);

/**
 * Returns true if the system is currently in display-dimmed or off state.
 */
bool PowerManager_is_display_off(void);

/**
 * Enters hardware deep sleep mode.
 * Used for manual shutdown or automated critical battery protection.
 */
void PowerManager_deep_sleep(void);

#endif