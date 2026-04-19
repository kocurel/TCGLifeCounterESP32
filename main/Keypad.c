#include "Keypad.h"

#include <rom/ets_sys.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "model/Button.h"

#define ROW_COUNT 3
#define COL_COUNT 3
#define TOTAL_KEYS (ROW_COUNT * COL_COUNT)
#define SCAN_INTERVAL_MS 5

/* Timing Thresholds based on 5ms scan ticks */
#define HOLD_THRESHOLD_TICKS 100  /* 500ms before auto-repeat starts */
#define TURBO_THRESHOLD_TICKS 240 /* 1.2s before entering high-speed mode */

/* Repeat Rates */
#define REPEAT_RATE_SLOW_TICKS 40 /* 200ms interval for initial repeat */
#define REPEAT_RATE_TURBO_TICKS 8 /* 40ms interval (~25Hz) for turbo repeat */

#define PRESS_MASK 0x07
#define RELEASE_MASK 0x07

/* Hardware GPIO mapping */
static const int ROW_PINS[ROW_COUNT] = {4, 5, 10};
static const int COL_PINS[COL_COUNT] = {2, 8, 9};

/* Keypad-to-ButtonCode mapping */
static const ButtonCode button_mapper[TOTAL_KEYS] = {
    BUTTON_CODE_CANCEL, BUTTON_CODE_SET,  BUTTON_CODE_ACCEPT,
    BUTTON_CODE_POWER,  BUTTON_CODE_MENU, BUTTON_CODE_RIGHT,
    BUTTON_CODE_UP,     BUTTON_CODE_LEFT, BUTTON_CODE_DOWN,
};

typedef struct {
    uint8_t history;     /* Debounce shift register */
    uint16_t hold_timer; /* Active press duration counter */
    bool is_held;        /* Logic state flag */
} KeyState;

static KeyState key_states[TOTAL_KEYS];
static QueueHandle_t keypad_queue;

QueueHandle_t get_keypad_queue() { return keypad_queue; }

/**
 * Dispatches button events to the queue.
 * Implements backpressure check to ensure UI responsiveness.
 */
static void send_key_event(const int key_index) {
    const ButtonCode code = button_mapper[key_index];

    if (uxQueueSpacesAvailable(keypad_queue) > 0) {
        xQueueSend(keypad_queue, &code, 0);
    }
}

/**
 * Hardware and state initialization
 */
void keypad_init() {
    /* Configure rows as Active-Low outputs */
    for (int i = 0; i < ROW_COUNT; i++) {
        gpio_reset_pin(ROW_PINS[i]);
        gpio_set_direction(ROW_PINS[i], GPIO_MODE_OUTPUT);
        gpio_set_level(ROW_PINS[i], 1);
    }

    /* Configure columns as inputs with internal pull-up */
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

/**
 * Background task for matrix scanning and event processing.
 * Manages debouncing, hold detection, and tiered auto-repeat logic.
 */
void keypad_scan_task(void* pvParameters) {
    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t interval_ticks = pdMS_TO_TICKS(SCAN_INTERVAL_MS);

    while (1) {
        bool current_scan_state[TOTAL_KEYS] = {false};

        /* Physical matrix scanning phase */
        for (int r = 0; r < ROW_COUNT; r++) {
            gpio_set_level(ROW_PINS[r], 0);
            ets_delay_us(10); /* Signal stabilization delay */

            for (int c = 0; c < COL_COUNT; c++) {
                const int key_idx = (r * COL_COUNT) + c;
                if (gpio_get_level(COL_PINS[c]) == 0) {
                    current_scan_state[key_idx] = true;
                }
            }
            gpio_set_level(ROW_PINS[r], 1);
        }

        /* Logical debounce and repeat processing */
        for (int i = 0; i < TOTAL_KEYS; i++) {
            KeyState* const k = &key_states[i];

            k->history = (k->history << 1) | (current_scan_state[i] ? 1 : 0);

            if (!k->is_held) {
                /* Press detection: match against bitmask */
                if ((k->history & PRESS_MASK) == PRESS_MASK) {
                    send_key_event(i);
                    k->is_held = true;
                    k->hold_timer = 0;
                }
            } else {
                /* Release detection: match against zero mask */
                if ((k->history & RELEASE_MASK) == 0) {
                    k->is_held = false;
                    k->hold_timer = 0;
                    xQueueReset(keypad_queue); /* Stop pending repeat events */
                } else {
                    /* Auto-repeat and turbo logic */
                    k->hold_timer++;
                    if (k->hold_timer >= HOLD_THRESHOLD_TICKS) {
                        const int repeat_delta =
                            k->hold_timer - HOLD_THRESHOLD_TICKS;
                        int current_rate = REPEAT_RATE_SLOW_TICKS;

                        if (k->hold_timer > TURBO_THRESHOLD_TICKS) {
                            current_rate = REPEAT_RATE_TURBO_TICKS;
                        }

                        if (repeat_delta % current_rate == 0) {
                            send_key_event(i);
                        }
                    }
                }
            }
        }

        vTaskDelayUntil(&last_wake_time, interval_ticks);
    }
}