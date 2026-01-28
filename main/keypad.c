#include "keypad.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

// Hardware Definitions - Keypad
#define ROW_COUNT 3
#define COL_COUNT 3

// Pin Mapping for Keypad
const int ROW_PINS[ROW_COUNT] = {4, 5, 10};
const int COL_PINS[COL_COUNT] = {2, 8, 9};

// Keymap definition
const char key_map[ROW_COUNT][COL_COUNT] = {
    {'1', '2', '3'}, {'4', '5', '6'}, {'7', '8', '9'}};

/**
 * Keys:
 * 1 rdown
 * 2 rup
 * 3 rright
 * 4 power
 * 5 rleft
 * 6 right
 * 7 up
 * 8 left
 * 9 down
 */

// Queue handle for inter-task communication
QueueHandle_t keypad_queue;

QueueHandle_t get_keypad_queue() { return keypad_queue; }

// Function to initialize the keypad
void keypad_init() {
    for (int i = 0; i < ROW_COUNT; i++) {
        gpio_reset_pin(ROW_PINS[i]);
        gpio_set_direction(ROW_PINS[i], GPIO_MODE_OUTPUT);
        gpio_set_level(ROW_PINS[i], 1);
    }

    for (int i = 0; i < COL_COUNT; i++) {
        gpio_reset_pin(COL_PINS[i]);
        gpio_set_direction(COL_PINS[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(COL_PINS[i], GPIO_PULLUP_ONLY);
    }

    keypad_queue = xQueueCreate(10, sizeof(char));
}

// Function to scan the keypad and send the pressed key
void keypad_scan_task(void* pvParameters) {
    while (1) {
        for (int r = 0; r < ROW_COUNT; r++) {
            gpio_set_level(ROW_PINS[r], 0);
            vTaskDelay(pdMS_TO_TICKS(5));

            for (int c = 0; c < COL_COUNT; c++) {
                if (gpio_get_level(COL_PINS[c]) == 0) {
                    char key = key_map[r][c];
                    xQueueSend(keypad_queue, &key, 0);
                    while (gpio_get_level(COL_PINS[c]) == 0) {
                        vTaskDelay(pdMS_TO_TICKS(20));
                    }
                }
            }
            gpio_set_level(ROW_PINS[r], 1);
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}