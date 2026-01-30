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
#include "gui_framework/include/test/GUItests.h"
#include "keypad.h"
#include "model/Game.h"
#include "model/Settings.h"
#include "model/Tests.h"

// Callback wywoływany, gdy użytkownik potwierdzi chęć wyłączenia
static void on_power_off_confirmed() {
    LOG_DEBUG("app_main", "Power off confirmed. Entering deep sleep.");
    PowerManager_deep_sleep();
}

void app_main(void) {
    // --- Hardware Wakeup / Init Logic ---
    // Release the holds from the previous deep sleep
    gpio_reset_pin(5);
    gpio_set_direction(5, GPIO_MODE_OUTPUT);
    gpio_set_level(5, 1);
    gpio_hold_dis(5);
    gpio_deep_sleep_hold_dis();

    gpio_reset_pin(21);
    gpio_set_direction(21, GPIO_MODE_OUTPUT);
    gpio_set_level(21, 0);
    gpio_hold_dis(21);

    // --- Subsystem Init ---
    PowerManager_init();  // Starts the background 20s check task

    LOG_DEBUG("app_main", "System initialized.");
    xTaskCreate(console_task, "console_cli", 5120, NULL, 5, NULL);

    LOG_DEBUG("app_main", "Initializing audio.");
    AudioManager_init();

    LOG_DEBUG("app_main", "Initializing display.");
    GUIRenderer_init();

    LOG_DEBUG("app_main", "Initializing keypad.");
    keypad_init();
    xTaskCreate(keypad_scan_task, "keypad_task", 2048, NULL, 5, NULL);

    LOG_DEBUG("app_main", "Initializing battery.");
    xTaskCreate(battery_scan_task, "battery_scan_task", 2048, NULL, 5, NULL);

    LOG_DEBUG("app_main", "Initializing UI.");
    Game_init();
    MainPage_enter();
    AudioManager_play_sound(SOUND_GAME_START);
    SettingsModel_init();

    ButtonCode received_key;

    // --- Main Event Loop ---
    // --- Main Event Loop ---
    while (1) {
        if (xQueueReceive(get_keypad_queue(), &received_key, portMAX_DELAY)) {
            bool was_sleeping = PowerManager_is_display_off();
            PowerManager_reset_timer();

            if (was_sleeping) {
                continue;
            }

            // --- ZMIANA TUTAJ ---
            if (received_key == BUTTON_CODE_POWER) {
                LOG_DEBUG("app_main",
                          "Power button pressed - showing confirm page");

                // Używamy naszej nowej strony potwierdzenia!
                // Jeśli użytkownik wybierze "Nie" (CANCEL), ConfirmPage
                // automatycznie wróci do MainPage.
                ConfirmPage_enter("Power off the device?",
                                  on_power_off_confirmed);

            } else {
                // Wszystkie inne przyciski idą do standardowego kontrolera
                // stron
                ViewController_button_handler(received_key);
            }
        }
    }
}