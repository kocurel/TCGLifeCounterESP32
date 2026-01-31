#include "System.h"

/* --- Private State --- */
static int battery_percentage = 0;

/* --- Accessors and Mutators --- */

/**
 * Updates the global battery state.
 * Expected range: 0-100.
 */
void System_set_battery_percentage(int state) { battery_percentage = state; }

/**
 * Returns the last measured battery percentage.
 */
int System_get_battery_percentage() { return battery_percentage; }