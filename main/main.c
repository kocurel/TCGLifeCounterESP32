#include <stdio.h>
#include <string.h>

#include "AudioManager.h"
#include "PowerManager.h"
#include "ViewController.h"
#include "app/pages/ConfirmPage.h"
#include "app/pages/MainPage.h"
#include "battery.h"
#include "console_init.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "keypad.h"
#include "model/Game.h"
#include "model/Settings.h"

/* --- Callbacks --- */

/**
 * Triggered via the ConfirmPage when the user confirms a manual shutdown.
 */
static void on_power_off_confirmed() {
    ESP_LOGI("app_main", "Power off confirmed. Entering deep sleep.");
    PowerManager_deep_sleep();
}

/* --- Main Application Entry --- */

void app_main(void) {
    /* --- 1. Hardware Wakeup & IO Restoration --- */

    // Release GPIO 5 (Main Power Hold) from previous Deep Sleep state
    gpio_reset_pin(5);
    gpio_set_direction(5, GPIO_MODE_OUTPUT);
    gpio_set_level(5, 1);
    gpio_hold_dis(5);
    gpio_deep_sleep_hold_dis();

    // Restore OLED Power (GPIO 21)
    gpio_reset_pin(21);
    gpio_set_direction(21, GPIO_MODE_OUTPUT);
    gpio_set_level(21, 0);
    gpio_hold_dis(21);

    /* --- 2. Subsystem Initialization --- */

    // Display and Settings
    GUIRenderer_init();
    SettingsModel_init();

    // Power and System Services
    PowerManager_init();  // Monitors activity and handles auto-sleep
    xTaskCreate(console_task, "console_cli", 5120, NULL, 5, NULL);
    AudioManager_init();

    // Input Peripherals
    keypad_init();
    xTaskCreate(keypad_scan_task, "keypad_task", 2048, NULL, 5, NULL);

    // Telemetry
    xTaskCreate(battery_scan_task, "battery_scan_task", 2048, NULL, 5, NULL);

    /* --- 3. UI State Initialization --- */

    Game_init();
    MainPage_enter();
    AudioManager_play_sound(SOUND_GAME_START);

    /* --- 4. Main Event Loop --- */

    ButtonCode received_key;
    uint32_t last_time = (uint32_t)(esp_timer_get_time() / 1000);
    const uint32_t tick_interval_ms = 20;  // Aiming for 50 FPS UI updates

    while (1) {
        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
        uint32_t delta_ms = now - last_time;

        // A. Logic Tick: Handle timed events and UI animations
        if (delta_ms >= tick_interval_ms) {
            PageManager_tick(delta_ms);
            last_time = now;
        }

        // B. Input Processing: Wait for keypad events with a timeout
        // Using the tick interval as the timeout ensures the loop keeps ticking
        if (xQueueReceive(get_keypad_queue(), &received_key,
                          pdMS_TO_TICKS(tick_interval_ms))) {
            bool was_sleeping = PowerManager_is_display_off();
            PowerManager_reset_timer();

            // If the device just woke up from Display Sleep, ignore the first
            // key press
            if (was_sleeping) {
                continue;
            }

            // Special Handling: Power Button (Global Action)
            if (received_key == BUTTON_CODE_POWER) {
                ConfirmPage_enter("Power off the device?",
                                  on_power_off_confirmed, MainPage_enter);
            }
            // Standard Handling: Forward to active page logic
            else {
                PageManager_handle_input(received_key);
            }
        }
    }
}