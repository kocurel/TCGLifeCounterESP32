#ifndef KEYPAD_H
#define KEYPAD_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

void keypad_init();
void keypad_scan_task(void* pvParameters);
QueueHandle_t get_keypad_queue();

#endif  // KEYPAD_H