#include "PowerManager.h"

#include "AudioManager.h"
#include "GUIRenderer.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "model/Settings.h"  // <--- Tutaj są nasze czasy

// Opcjonalne PM
#include "esp_pm.h"

static const char* TAG = "PowerManager";
static int64_t last_activity_time = 0;
static bool s_display_is_off = false;

// Helper czasu (ms od startu)
static int64_t get_time_ms() { return esp_timer_get_time() / 1000; }

// LEVEL 1: Display Sleep (Soft Off)
static void enter_display_sleep() {
    if (s_display_is_off) return;

    ESP_LOGI(TAG, "Timeout: Entering Display Sleep...");

    // 1. Wyłącz renderowanie (OLED Sleep Command)
    GUIRenderer_power_save_enable();

    // 2. Fizyczne odcięcie zasilania ekranu (zgodnie z Twoim PCB)
    gpio_set_level(21, 1);

    s_display_is_off = true;
}

static void exit_display_sleep() {
    if (!s_display_is_off) return;

    ESP_LOGI(TAG, "Activity: Waking up Display...");

    // 1. Włącz zasilanie ekranu
    gpio_set_level(21, 0);

    // Daj chwilę przetwornicy na ustabilizowanie napięcia
    vTaskDelay(pdMS_TO_TICKS(50));

    // 2. Wybudź OLED logicznie
    GUIRenderer_power_save_disable();

    // 3. WAŻNE: Przywróć jasność ustawioną przez użytkownika!
    // Po resecie zasilania OLED może wstać z domyślnym (ciemnym) kontrastem.
    GameSettings settings = SettingsModel_get();
    GUIRenderer_set_contrast(settings.screen_brightness);

    s_display_is_off = false;
}

// LEVEL 2: Deep Sleep (Hard Kill)
static void enter_deep_sleep() {
    ESP_LOGI(TAG, "Timeout: Initiating Deep Sleep...");

    // Dźwięk pożegnalny (opcjonalnie)
    AudioManager_play_sound(SOUND_UI_CANCEL);
    vTaskDelay(pdMS_TO_TICKS(500));  // Czekamy aż dźwięk wybrzmi

    // Zabezpieczenie buzzera (musi być cicho w deep sleep)
    ledc_stop(BUZZER_MODE, BUZZER_CHANNEL, 0);

    // --- Konfiguracja pinów na czas snu (HOLD) ---

    // GPIO 5 (External Control?) -> LOW
    gpio_reset_pin(5);
    gpio_set_direction(5, GPIO_MODE_OUTPUT);
    gpio_set_level(5, 0);
    gpio_hold_en(5);

    // GPIO 21 (OLED Power) -> HIGH (OFF)
    gpio_reset_pin(21);
    gpio_set_direction(21, GPIO_MODE_OUTPUT);
    gpio_set_level(21, 1);
    gpio_hold_en(21);

    // Globalne włączenie podtrzymania pinów w deep sleep
    gpio_deep_sleep_hold_en();

    // --- Konfiguracja wybudzania ---

    // Wybudzanie dowolnym klawiszem (zakładamy, że podłączony do GPIO 2 z
    // Pull-up) Dostosuj numer pinu, jeśli Twój główny przycisk wybudzania jest
    // inny!
    const int WAKEUP_PIN = 2;
    gpio_reset_pin(WAKEUP_PIN);
    gpio_set_direction(WAKEUP_PIN, GPIO_MODE_INPUT);
    gpio_pullup_en(WAKEUP_PIN);
    esp_deep_sleep_enable_gpio_wakeup(1ULL << WAKEUP_PIN,
                                      ESP_GPIO_WAKEUP_GPIO_LOW);

    ESP_LOGI(TAG, "Goodbye. See you on the other side.");
    esp_deep_sleep_start();
}

void PowerManager_deep_sleep() { enter_deep_sleep(); }

void PowerManager_reset_timer() {
    // Aktualizacja czasu ostatniej aktywności
    last_activity_time = get_time_ms();

    // Jeśli ekran był wygaszony (Level 1), to go budzimy
    if (s_display_is_off) {
        exit_display_sleep();
    }
}

bool PowerManager_is_display_off() { return s_display_is_off; }

// Główny wątek sprawdzający czasy
static void power_manager_task(void* arg) {
    while (1) {
        // Sprawdzamy co 1 sekundę (częściej = lepsza precyzja, rzadziej =
        // mniejsze zużycie CPU)
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Pobieramy aktualne ustawienia
        GameSettings settings = SettingsModel_get();

        // Konwersja minut na milisekundy
        int64_t dim_timeout_ms =
            (int64_t)settings.screen_timeout_min * 60 * 1000;
        int64_t off_timeout_ms = (int64_t)settings.auto_off_min * 60 * 1000;

        int64_t now = get_time_ms();
        int64_t diff = now - last_activity_time;

        // Priorytety: Najpierw sprawdzamy Deep Sleep (bo jest ważniejszy)
        if (off_timeout_ms > 0 && diff > off_timeout_ms) {
            enter_deep_sleep();
        }
        // Potem sprawdzamy wygaszenie ekranu
        else if (dim_timeout_ms > 0 && diff > dim_timeout_ms) {
            enter_display_sleep();
        }
    }
}

void PowerManager_init() {
    last_activity_time = get_time_ms();
    s_display_is_off = false;

    // Konfiguracja pinu zasilania ekranu
    gpio_reset_pin(21);
    gpio_set_direction(21, GPIO_MODE_OUTPUT);
    gpio_set_level(21, 0);  // Start: Ekran włączony (Active Low logic?)
                            // UWAGA: Jeśli 21=HIGH wyłącza, to 21=0 włącza.

    // Opcjonalnie: Dynamic Frequency Scaling (DFS)
    // Pozwala zejść z taktowaniem CPU, gdy system "nic nie robi" (idle task)
#if CONFIG_PM_ENABLE
    esp_pm_config_esp32c3_t pm_config = {
        .max_freq_mhz = 160,
        .min_freq_mhz = 10,         // W idle schodzimy do 10MHz
        .light_sleep_enable = true  // Pozwól na Light Sleep w vTaskDelay
    };
    esp_pm_configure(&pm_config);
#endif

    // Uruchomienie strażnika czasu
    xTaskCreate(power_manager_task, "PowerMgr", 2048, NULL, 1, NULL);

    ESP_LOGI(TAG, "Initialized.");
}