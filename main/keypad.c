#include "keypad.h"

#include <rom/ets_sys.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "model/Button.h"

// --- Configuration ---
#define ROW_COUNT 3
#define COL_COUNT 3
#define TOTAL_KEYS (ROW_COUNT * COL_COUNT)

// 5ms is the "Sweet Spot" for responsiveness vs CPU load
#define SCAN_INTERVAL_MS 5

// Timings adjusted for 5ms ticks
// 500ms / 5ms = 100 ticks
#define HOLD_THRESHOLD_TICKS 100
// 200ms / 5ms = 40 ticks
#define REPEAT_RATE_SLOW_TICKS 40
// 60ms / 5ms = 12 ticks
#define REPEAT_RATE_FAST_TICKS 12
// 2000ms / 5ms = 400 ticks
#define TURBO_THRESHOLD_TICKS 400

// Debounce Masks
// 0x07 = 0000 0111 (Requires 3 stable ticks / 15ms to trigger)
#define PRESS_MASK 0x07
// 0x07 = 0000 0111 (Requires 3 stable empty ticks / 15ms to release)
#define RELEASE_MASK 0x07

// Hardware Pins
const int ROW_PINS[ROW_COUNT] = {4, 5, 10};
const int COL_PINS[COL_COUNT] = {2, 8, 9};

// Key Mapping
const ButtonCode button_mapper[TOTAL_KEYS] = {
    BUTTON_CODE_CANCEL, BUTTON_CODE_SET,  BUTTON_CODE_ACCEPT,
    BUTTON_CODE_POWER,  BUTTON_CODE_MENU, BUTTON_CODE_RIGHT,
    BUTTON_CODE_UP,     BUTTON_CODE_LEFT, BUTTON_CODE_DOWN,
};

// --- State Management ---
typedef struct {
    uint8_t history;      // 8-bit shift register
    uint16_t hold_timer;  // Counts 5ms ticks
    bool is_held;         // Locked state flag
} KeyState;

static KeyState key_states[TOTAL_KEYS];
QueueHandle_t keypad_queue;

QueueHandle_t get_keypad_queue() { return keypad_queue; }

void keypad_init() {
    // 1. Initialize Rows (Outputs, Active LOW logic, so start HIGH)
    for (int i = 0; i < ROW_COUNT; i++) {
        gpio_reset_pin(ROW_PINS[i]);
        gpio_set_direction(ROW_PINS[i], GPIO_MODE_OUTPUT);
        gpio_set_level(ROW_PINS[i], 1);
    }

    // 2. Initialize Cols (Inputs, Pull-up)
    for (int i = 0; i < COL_COUNT; i++) {
        gpio_reset_pin(COL_PINS[i]);
        gpio_set_direction(COL_PINS[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(COL_PINS[i], GPIO_PULLUP_ONLY);
    }

    // 3. Reset State
    for (int i = 0; i < TOTAL_KEYS; i++) {
        key_states[i].history = 0;
        key_states[i].hold_timer = 0;
        key_states[i].is_held = false;
    }

    keypad_queue = xQueueCreate(10, sizeof(ButtonCode));
}

static void send_key_event(int key_index) {
    ButtonCode code = button_mapper[key_index];
    xQueueSend(keypad_queue, &code, 0);
}

void keypad_scan_task(void* pvParameters) {
    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t interval_ticks = pdMS_TO_TICKS(SCAN_INTERVAL_MS);

    while (1) {
        // --- 1. Hardware Scan ---
        bool current_scan_state[TOTAL_KEYS] = {false};

        for (int r = 0; r < ROW_COUNT; r++) {
            // Drive Row LOW
            gpio_set_level(ROW_PINS[r], 0);
            ets_delay_us(10);  // Signal settling time

            for (int c = 0; c < COL_COUNT; c++) {
                int key_idx = (r * COL_COUNT) + c;
                // Check Col (Active LOW)
                if (gpio_get_level(COL_PINS[c]) == 0) {
                    current_scan_state[key_idx] = true;
                }
            }
            // Drive Row HIGH
            gpio_set_level(ROW_PINS[r], 1);
        }

        // --- 2. Logic Processing ---
        for (int i = 0; i < TOTAL_KEYS; i++) {
            KeyState* k = &key_states[i];

            // Shift history left, append new state
            k->history = (k->history << 1) | (current_scan_state[i] ? 1 : 0);

            // LOGIC: State Machine with Hysteresis

            if (k->is_held == false) {
                // --- DETECT PRESS ---
                // We check if the last 3 bits are 1 (0x07).
                // We DO NOT check for leading 0s. This fixes "missed presses"
                // caused by bounce noise.
                if ((k->history & PRESS_MASK) == PRESS_MASK) {
                    send_key_event(i);
                    k->is_held = true;
                    k->hold_timer = 0;
                }
            } else {
                // --- DETECT RELEASE ---
                // Key is currently held. We wait for 3 consecutive 0s to
                // consider it released.
                if ((k->history & RELEASE_MASK) == 0) {
                    k->is_held = false;
                    k->hold_timer = 0;
                }
                // --- HANDLE HOLD REPEAT ---
                else {
                    k->hold_timer++;
                    if (k->hold_timer >= HOLD_THRESHOLD_TICKS) {
                        int repeat_time = k->hold_timer - HOLD_THRESHOLD_TICKS;

                        // Turbo Logic
                        int current_rate = REPEAT_RATE_SLOW_TICKS;
                        if (k->hold_timer > TURBO_THRESHOLD_TICKS) {
                            current_rate = REPEAT_RATE_FAST_TICKS;
                        }

                        if (repeat_time % current_rate == 0) {
                            send_key_event(i);
                        }
                    }
                }
            }
        }

        // --- 3. Strict Timing ---
        vTaskDelayUntil(&last_wake_time, interval_ticks);
    }
}