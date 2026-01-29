#ifndef BATTERY_H
#define BATTERY_H

void read_battery_level();
void battery_scan_task(void* pvParameters);

#endif  // BATTERY_H