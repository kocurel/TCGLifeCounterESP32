#include "Settings.h"

#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char* TAG = "SettingsModel";
static const char* NVS_NAMESPACE = "game_config";

// Cache ustawień w RAM (dzięki temu gra działa szybko i nie czyta Flasha co
// chwilę)
static GameSettings s_current_settings;

// Domyślne wartości, jeśli NVS jest pusty (np. pierwsze uruchomienie)
static const GameSettings DEFAULT_SETTINGS = {.starting_life = 40,
                                              .starting_player_count = 4,
                                              .screen_brightness = 80,
                                              .sound_enabled = true};

void SettingsModel_init() {
    // 1. Inicjalizacja partycji NVS (wymagane raz w app_main, ale bezpiecznie
    // tu sprawdzić)
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // 2. Otwarcie Namespace
    nvs_handle_t my_handle;
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &my_handle);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        s_current_settings = DEFAULT_SETTINGS;
        return;
    }

    // 3. Odczyt wartości
    // Jeśli klucz nie istnieje (ESP_ERR_NVS_NOT_FOUND), używamy domyślnej
    uint8_t val_u8;

    // Starting Life
    if (nvs_get_u8(my_handle, "start_life", &val_u8) == ESP_OK)
        s_current_settings.starting_life = val_u8;
    else
        s_current_settings.starting_life = DEFAULT_SETTINGS.starting_life;

    // Player Count
    if (nvs_get_u8(my_handle, "p_count", &val_u8) == ESP_OK)
        s_current_settings.starting_player_count = val_u8;
    else
        s_current_settings.starting_player_count =
            DEFAULT_SETTINGS.starting_player_count;

    // Sound
    if (nvs_get_u8(my_handle, "sound", &val_u8) == ESP_OK)
        s_current_settings.sound_enabled = (bool)val_u8;
    else
        s_current_settings.sound_enabled = DEFAULT_SETTINGS.sound_enabled;

    // Zamykamy uchwyt
    nvs_close(my_handle);

    ESP_LOGI(TAG, "Settings Loaded: Life=%d, Players=%d",
             s_current_settings.starting_life,
             s_current_settings.starting_player_count);
}

GameSettings SettingsModel_get() { return s_current_settings; }

void SettingsModel_save(GameSettings new_settings) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return;

    // Aktualizujemy NVS tylko jeśli wartości się zmieniły (oszczędzanie flasha)
    if (new_settings.starting_life != s_current_settings.starting_life)
        nvs_set_u8(my_handle, "start_life", new_settings.starting_life);

    if (new_settings.starting_player_count !=
        s_current_settings.starting_player_count)
        nvs_set_u8(my_handle, "p_count", new_settings.starting_player_count);

    if (new_settings.sound_enabled != s_current_settings.sound_enabled)
        nvs_set_u8(my_handle, "sound", (uint8_t)new_settings.sound_enabled);

    // Commit jest konieczny, aby zapisać zmiany fizycznie
    err = nvs_commit(my_handle);
    nvs_close(my_handle);

    // Aktualizujemy cache w RAM
    s_current_settings = new_settings;

    ESP_LOGI(TAG, "Settings Saved");
}