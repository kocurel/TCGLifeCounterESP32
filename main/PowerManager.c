#include "PowerManager.h"

#include "AudioManager.h"
#include "GUIRenderer.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// Dla zaawansowanego PM (opcjonalne, wymaga menuconfig)
#include "AudioManager.h"
#include "driver/ledc.h"
#include "esp_pm.h"

static const char* TAG = "PowerManager";
static int64_t last_activity_time = 0;
static bool s_display_is_off = false;

// Helper czasu
static int64_t get_time_ms() { return esp_timer_get_time() / 1000; }

// LEVEL 1: Display Sleep (Soft Off)
static void enter_display_sleep() {
    if (s_display_is_off) return;

    ESP_LOGI(TAG, "Entering Display Sleep (Level 1)...");

    // 1. Wyłącz renderowanie OLED (komenda 0xAE w u8g2)
    GUIRenderer_power_save_enable();

    // 2. Odłącz zasilanie ekranu (GPIO 21 HIGH - zgodnie z Twoim opisem)
    gpio_set_level(21, 1);

    s_display_is_off = true;

    // Opcjonalnie: Możemy tu zwolnić locka PM, żeby pozwolić na głębszy light
    // sleep
}

static void exit_display_sleep() {
    if (!s_display_is_off) return;

    ESP_LOGI(TAG, "Waking up Display...");

    // 1. Włącz zasilanie ekranu
    gpio_set_level(21, 0);

    // Daj chwilę na ustabilizowanie napięcia
    vTaskDelay(pdMS_TO_TICKS(50));

    // 2. Wybudź OLED logicznie
    GUIRenderer_power_save_disable();

    s_display_is_off = false;
}

// LEVEL 2: Deep Sleep (Hard Kill)
static void enter_deep_sleep() {
    ESP_LOGI(TAG, "Initiating Deep Sleep (Level 2)...");

    AudioManager_play_sound(SOUND_UI_CANCEL);
    vTaskDelay(pdMS_TO_TICKS(600));

    // Utrzymanie stanu pinów (żeby ekran nie błysnął przy wyłączaniu)
    // GPIO 5 HIGH (zakładam, że to sterowanie czymś innym?)
    gpio_reset_pin(5);
    gpio_set_direction(5, GPIO_MODE_OUTPUT);
    gpio_set_level(5, 0);
    gpio_hold_en(5);

    // GPIO 21 HIGH (Ekran OFF)
    gpio_reset_pin(21);
    gpio_set_direction(21, GPIO_MODE_OUTPUT);
    gpio_set_level(21, 1);
    gpio_hold_en(21);
    gpio_deep_sleep_hold_en();  // Globalne włączenie holda w deep sleep

    // Wakeup source: GPIO 2 (Keypad COL 0 - przykładowy klawisz)
    gpio_reset_pin(2);
    gpio_set_direction(2, GPIO_MODE_INPUT);
    gpio_pullup_en(2);
    esp_deep_sleep_enable_gpio_wakeup(1ULL << 2, ESP_GPIO_WAKEUP_GPIO_LOW);

    ledc_stop(BUZZER_MODE, BUZZER_CHANNEL, 0);

    esp_deep_sleep_start();
}

void PowerManager_deep_sleep() { enter_deep_sleep(); }

void PowerManager_reset_timer() {
    last_activity_time = get_time_ms();

    // Jeśli zresetowano timer, a ekran spał -> obudź go!
    if (s_display_is_off) {
        exit_display_sleep();
    }
}

bool PowerManager_is_display_off() { return s_display_is_off; }

// Background Task
static void power_manager_task(void* arg) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000));  // Sprawdzaj co 5 sekund

        int64_t diff = get_time_ms() - last_activity_time;

        if (diff > POWER_TIMEOUT_DEEP_SLEEP_MS) {
            enter_deep_sleep();
        } else if (diff > POWER_TIMEOUT_DISPLAY_MS) {
            enter_display_sleep();
        }
    }
}

void PowerManager_init() {
    last_activity_time = get_time_ms();
    s_display_is_off = false;

    // Konfiguracja pinów zasilania
    gpio_reset_pin(21);
    gpio_set_direction(21, GPIO_MODE_OUTPUT);
    gpio_set_level(21, 0);  // Start: Ekran włączony

// Opcjonalnie: Konfiguracja dynamicznego skalowania częstotliwości (Level 1.5)
#if CONFIG_PM_ENABLE
    esp_pm_config_esp32c3_t pm_config = {
        .max_freq_mhz = 160,
        .min_freq_mhz = 10,         // Zwolnij do 10MHz gdy nic nie robisz
        .light_sleep_enable = true  // Pozwól na Light Sleep w vTaskDelay
    };
    esp_pm_configure(&pm_config);
#endif

    xTaskCreate(power_manager_task, "PowerMgr", 2048, NULL, 1, NULL);
}