#include "PowerManager.h"

#include "AudioManager.h"
#include "GUIRenderer.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_pm.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "model/Settings.h"

static const char* TAG = "PowerManager";

/* --- Private State --- */
static int64_t last_activity_time = 0;
static bool s_display_is_off = false;

/* --- Internal Helpers --- */

static int64_t get_time_ms() { return esp_timer_get_time() / 1000; }

static void enter_display_sleep() {
    if (s_display_is_off) return;

    ESP_LOGI(TAG, "Entering Display Sleep...");

    // 1. Soft sleep for OLED controller
    GUIRenderer_power_save_enable();

    // 2. Cut physical power (GPIO 21)
    gpio_set_level(21, 1);

    s_display_is_off = true;
}

static void exit_display_sleep() {
    if (!s_display_is_off) return;

    ESP_LOGI(TAG, "Waking up Display...");

    // 1. Restore physical power
    gpio_set_level(21, 0);
    vTaskDelay(pdMS_TO_TICKS(50));

    // 2. Logic wake up
    GUIRenderer_power_save_disable();

    // 3. Restore user contrast
    GameSettings settings = SettingsModel_get();
    GUIRenderer_set_contrast(settings.screen_brightness);

    s_display_is_off = false;
}

static void enter_deep_sleep() {
    ESP_LOGI(TAG, "Initiating Deep Sleep...");

    AudioManager_play_sound(SOUND_UI_CANCEL);
    vTaskDelay(pdMS_TO_TICKS(500));

    ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);

    // --- GPIO Hold Configuration ---
    gpio_reset_pin(5);
    gpio_set_direction(5, GPIO_MODE_OUTPUT);
    gpio_set_level(5, 0);
    gpio_hold_en(5);

    gpio_reset_pin(21);
    gpio_set_direction(21, GPIO_MODE_OUTPUT);
    gpio_set_level(21, 1);
    gpio_hold_en(21);

    gpio_deep_sleep_hold_en();

    // --- Wakeup Configuration ---
    const int WAKEUP_PIN = 2;
    gpio_reset_pin(WAKEUP_PIN);
    gpio_set_direction(WAKEUP_PIN, GPIO_MODE_INPUT);
    gpio_pullup_en(WAKEUP_PIN);
    esp_deep_sleep_enable_gpio_wakeup(1ULL << WAKEUP_PIN,
                                      ESP_GPIO_WAKEUP_GPIO_LOW);

    ESP_LOGI(TAG, "Entering Deep Sleep now.");
    esp_deep_sleep_start();
}

/* --- Public API --- */

void PowerManager_deep_sleep() { enter_deep_sleep(); }

void PowerManager_reset_timer() {
    last_activity_time = get_time_ms();
    if (s_display_is_off) {
        exit_display_sleep();
    }
}

bool PowerManager_is_display_off() { return s_display_is_off; }

/* --- Tasks --- */

static void power_manager_task(void* arg) {
    // Cache settings to avoid constant NVS access in the loop
    GameSettings settings = SettingsModel_get();
    int64_t last_settings_check = 0;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Refresh settings from NVS every 5 seconds or upon activity
        int64_t now = get_time_ms();
        if (now - last_settings_check > 5000) {
            settings = SettingsModel_get();
            last_settings_check = now;
        }

        int64_t dim_timeout_ms =
            (int64_t)settings.screen_timeout_min * 60 * 1000;
        int64_t off_timeout_ms = (int64_t)settings.auto_off_min * 60 * 1000;
        int64_t diff = now - last_activity_time;

        if (off_timeout_ms > 0 && diff > off_timeout_ms) {
            enter_deep_sleep();
        } else if (dim_timeout_ms > 0 && diff > dim_timeout_ms) {
            enter_display_sleep();
        }
    }
}

/* --- Initialization --- */

void PowerManager_init() {
    last_activity_time = get_time_ms();
    s_display_is_off = false;

    gpio_reset_pin(21);
    gpio_set_direction(21, GPIO_MODE_OUTPUT);
    gpio_set_level(21, 0);

#if CONFIG_PM_ENABLE
    esp_pm_config_esp32c3_t pm_config = {
        .max_freq_mhz = 160, .min_freq_mhz = 10, .light_sleep_enable = true};
    esp_pm_configure(&pm_config);
#endif

    // Increased stack size to 3072 to prevent Stack Protection Fault
    xTaskCreate(power_manager_task, "PowerMgr", 3072, NULL, 1, NULL);

    ESP_LOGI(TAG, "Initialized with 3KB stack.");
}