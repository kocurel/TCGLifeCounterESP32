#include "System.h"

static int battery_percentage = 0;

void System_set_battery_percentage(int state) { battery_percentage = state; }

int System_get_battery_percentage() { return battery_percentage; }
