#ifndef KEYPAD_H
#define KEYPAD_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define DPAD_DOWN 0x01
#define DPAD_UP 0x02
#define DPAD_LEFT 0x03
#define DPAD_RIGHT 0x04
#define ACTION_DOWN 0x05
#define ACTION_UP 0x06
#define ACTION_LEFT 0x07
#define ACTION_RIGHT 0x08
#define ACTION_POWER 0x09

void keypad_init();
void keypad_scan_task(void* pvParameters);
QueueHandle_t get_keypad_queue();

#endif  // KEYPAD_H