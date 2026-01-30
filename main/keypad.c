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

#define SCAN_INTERVAL_MS 5

// Ticks (5ms base)
#define HOLD_THRESHOLD_TICKS 100   // 500ms do rozpoczęcia powtarzania
#define TURBO_THRESHOLD_TICKS 240  // 1.2s do wejścia w turbo

// --- Szybkość powtarzania (Tuning) ---
#define REPEAT_RATE_SLOW_TICKS 40  // 200ms między zmianami (początek)
// 8 ticków * 5ms = 40ms (~25Hz). To bezpieczny limit dla I2C i OLED
// przy pełnym odświeżaniu ekranu, eliminujący lag.
#define REPEAT_RATE_TURBO_TICKS 8

// Debounce Masks
#define PRESS_MASK 0x07
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
    for (int i = 0; i < TOTAL_KEYS; i++) {
        key_states[i].history = 0;
        key_states[i].hold_timer = 0;
        key_states[i].is_held = false;
    }

    keypad_queue = xQueueCreate(10, sizeof(ButtonCode));
}

static void send_key_event(int key_index) {
    ButtonCode code = button_mapper[key_index];

    // Backpressure prevention: jeśli kolejka pełna, ignorujemy powtórzenia
    // Turbo
    if (uxQueueSpacesAvailable(keypad_queue) > 0) {
        xQueueSend(keypad_queue, &code, 0);
    }
}

void keypad_scan_task(void* pvParameters) {
    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t interval_ticks = pdMS_TO_TICKS(SCAN_INTERVAL_MS);

    while (1) {
        // --- 1. Hardware Scan ---
        bool current_scan_state[TOTAL_KEYS] = {false};

        for (int r = 0; r < ROW_COUNT; r++) {
            gpio_set_level(ROW_PINS[r], 0);
            ets_delay_us(10);  // Signal settling time

            for (int c = 0; c < COL_COUNT; c++) {
                int key_idx = (r * COL_COUNT) + c;
                if (gpio_get_level(COL_PINS[c]) == 0) {
                    current_scan_state[key_idx] = true;
                }
            }
            gpio_set_level(ROW_PINS[r], 1);
        }

        // --- 2. Logic Processing ---
        for (int i = 0; i < TOTAL_KEYS; i++) {
            KeyState* k = &key_states[i];
            k->history = (k->history << 1) | (current_scan_state[i] ? 1 : 0);

            if (k->is_held == false) {
                // DETECT PRESS
                if ((k->history & PRESS_MASK) == PRESS_MASK) {
                    send_key_event(i);
                    k->is_held = true;
                    k->hold_timer = 0;
                }
            } else {
                // DETECT RELEASE
                if ((k->history & RELEASE_MASK) == 0) {
                    k->is_held = false;
                    k->hold_timer = 0;

                    // Hamulec bezpieczeństwa: czyścimy kolejkę przy puszczeniu
                    // klawisza
                    xQueueReset(keypad_queue);
                }
                // HANDLE HOLD REPEAT
                else {
                    k->hold_timer++;
                    if (k->hold_timer >= HOLD_THRESHOLD_TICKS) {
                        int repeat_time = k->hold_timer - HOLD_THRESHOLD_TICKS;

                        int current_ticks = REPEAT_RATE_SLOW_TICKS;
                        if (k->hold_timer > TURBO_THRESHOLD_TICKS) {
                            current_ticks = REPEAT_RATE_TURBO_TICKS;
                        }

                        if (repeat_time % current_ticks == 0) {
                            send_key_event(i);
                        }
                    }
                }
            }
        }
        vTaskDelayUntil(&last_wake_time, interval_ticks);
    }
}