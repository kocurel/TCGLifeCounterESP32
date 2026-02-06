#include <stdio.h>
#include <string.h>

#include "AudioManager.h"
#include "Keypad.h"
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
    int64_t last_frame_time = esp_timer_get_time();
    const int64_t frame_duration_us = 20000;  // 20ms = 20000us (50 FPS)

    while (1) {
        // 1. Obliczamy Delta Time (w ms dla logiki)
        int64_t now = esp_timer_get_time();
        int64_t elapsed = now - last_frame_time;

        // Jeśli minęło więcej niż 20ms, rysujemy klatkę
        if (elapsed >= frame_duration_us) {
            // Przekazujemy rzeczywisty czas (np. 21ms, 35ms) do fizyki
            PageManager_tick((uint32_t)(elapsed / 1000));

            // Przesuwamy punkt odniesienia o "czas klatki", żeby utrzymać rytm
            // (chyba że lag jest ogromny, wtedy resetujemy do 'now')
            last_frame_time = now;
        }

        // 2. Obliczamy, ile czasu ZOSTAŁO do następnej klatki
        // Żeby nie obciążać CPU, śpimy tylko tyle, ile trzeba
        now = esp_timer_get_time();
        int64_t work_done = now - last_frame_time;
        int64_t time_to_wait_us = frame_duration_us - work_done;

        // Konwersja na Tick FreeRTOS (z zabezpieczeniem przed ujemnym czasem)
        TickType_t wait_ticks = 0;
        if (time_to_wait_us > 0) {
            wait_ticks = pdMS_TO_TICKS(time_to_wait_us / 1000);
        }

        // 3. Czekamy na Input (tylko tyle ile mamy wolnego czasu)
        if (xQueueReceive(get_keypad_queue(), &received_key, wait_ticks)) {
            bool was_sleeping = PowerManager_is_display_off();
            PowerManager_reset_timer();

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