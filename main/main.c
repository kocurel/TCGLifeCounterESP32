#include <stdio.h>

#include "AudioManager.h"
#include "GUIRenderer.h"
#include "Keypad.h"
#include "PowerManager.h"
#include "app/PageManager.h"
#include "app/pages/ConfirmPage.h"
#include "app/pages/MainPage.h"
#include "battery.h"
#include "console_init.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "model/Button.h"
#include "model/Game.h"
#include "model/Settings.h"

/**
 * Handles system shutdown sequence upon user confirmation
 */
static void on_power_off_confirmed() {
    ESP_LOGI("app_main", "Power off confirmed. Entering deep sleep.");
    PowerManager_deep_sleep();
}

/**
 * Main application entry point.
 * Manages hardware restoration, task spawning, and the primary event loop.
 */
void app_main(void) {
    /* Restore GPIO states and release pins from Deep Sleep hold mode */
    gpio_reset_pin(5);
    gpio_set_direction(5, GPIO_MODE_OUTPUT);
    gpio_set_level(5, 1);
    gpio_hold_dis(5);
    gpio_deep_sleep_hold_dis();

    /* Restore OLED power rails */
    gpio_reset_pin(21);
    gpio_set_direction(21, GPIO_MODE_OUTPUT);
    gpio_set_level(21, 0);
    gpio_hold_dis(21);

    /* Initialize core subsystems and hardware drivers */
    GUIRenderer_init();
    SettingsModel_init();
    PowerManager_init();
    AudioManager_init();
    keypad_init();

    /* Spawn background service tasks */
    xTaskCreate(console_task, "console_cli", 5120, NULL, 5, NULL);
    xTaskCreate(keypad_scan_task, "keypad_task", 2048, NULL, 5, NULL);
    xTaskCreate(battery_scan_task, "battery_scan_task", 2048, NULL, 5, NULL);

    /* Initialize application state and enter primary UI page */
    Game_init();
    MainPage_enter();
    AudioManager_play_sound(SOUND_GAME_START);

    /* Primary Event Loop: Manages frame timing and input dispatching */
    ButtonCode received_key;
    int64_t last_frame_time = esp_timer_get_time();
    const int64_t frame_duration_us = 20000; /* Target: 50 FPS */

    while (1) {
        int64_t now = esp_timer_get_time();
        const int64_t elapsed = now - last_frame_time;

        /* Process frame logic updates on timing match */
        if (elapsed >= frame_duration_us) {
            PageManager_tick((uint32_t)(elapsed / 1000));
            last_frame_time = now;
        }

        /* Calculate remaining idle time for high-efficiency queue waiting */
        now = esp_timer_get_time();
        const int64_t work_done = now - last_frame_time;
        const int64_t time_to_wait_us = frame_duration_us - work_done;

        TickType_t wait_ticks = 0;
        if (time_to_wait_us > 0) {
            wait_ticks = pdMS_TO_TICKS(time_to_wait_us / 1000);
        }

        /* Input processing with display wake-up logic */
        if (xQueueReceive(get_keypad_queue(), &received_key, wait_ticks)) {
            const bool was_sleeping = PowerManager_is_display_off();
            PowerManager_reset_timer();

            /* Suppress first input event if it was used only to wake the
             * display */
            if (was_sleeping) continue;

            if (received_key == BUTTON_CODE_POWER) {
                ConfirmPage_enter("Power off?", on_power_off_confirmed,
                                  MainPage_enter);
            } else {
                PageManager_handle_input(received_key);
            }
        }
    }
}