#include "Settings.h"

#include "GUIRenderer.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char* TAG = "SettingsModel";
static const char* NVS_NAMESPACE = "game_config";

static GameSettings s_current_settings;

static const GameSettings DEFAULT_SETTINGS = {.starting_life = 40,
                                              .screen_brightness = 32,
                                              .sound_loudness = 50,
                                              .screen_timeout_min = 2,
                                              .auto_off_min = 15,
                                              .sound_enabled = true,
                                              .dead_at_zero = true,
                                              .cmd_dmg_rule = true};

void SettingsModel_init() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    nvs_handle_t handle;
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        s_current_settings = DEFAULT_SETTINGS;
        return;
    }

    uint8_t val8;
    uint16_t val16;

    // Odczyt Starting Life (uint16)
    if (nvs_get_u16(handle, "start_life", &val16) != ESP_OK)
        val16 = DEFAULT_SETTINGS.starting_life;
    s_current_settings.starting_life = val16;

    // Odczyt parametrów 8-bitowych
    if (nvs_get_u8(handle, "scr_br", &val8) != ESP_OK)
        val8 = DEFAULT_SETTINGS.screen_brightness;
    s_current_settings.screen_brightness = val8;

    if (nvs_get_u8(handle, "loud", &val8) != ESP_OK)
        val8 = DEFAULT_SETTINGS.sound_loudness;
    s_current_settings.sound_loudness = val8;

    if (nvs_get_u8(handle, "dim_t", &val8) != ESP_OK)
        val8 = DEFAULT_SETTINGS.screen_timeout_min;
    s_current_settings.screen_timeout_min = val8;

    if (nvs_get_u8(handle, "off_t", &val8) != ESP_OK)
        val8 = DEFAULT_SETTINGS.auto_off_min;
    s_current_settings.auto_off_min = val8;

    if (nvs_get_u8(handle, "snd_en", &val8) != ESP_OK)
        val8 = (uint8_t)DEFAULT_SETTINGS.sound_enabled;
    s_current_settings.sound_enabled = (bool)val8;

    if (nvs_get_u8(handle, "dead0", &val8) != ESP_OK)
        val8 = (uint8_t)DEFAULT_SETTINGS.dead_at_zero;
    s_current_settings.dead_at_zero = (bool)val8;

    if (nvs_get_u8(handle, "cmd21", &val8) != ESP_OK)
        val8 = (uint8_t)DEFAULT_SETTINGS.cmd_dmg_rule;
    s_current_settings.cmd_dmg_rule = (bool)val8;

    nvs_close(handle);

    GUIRenderer_set_contrast(s_current_settings.screen_brightness);
    ESP_LOGI(TAG, "Settings initialized and cleaned.");
}

void SettingsModel_save(GameSettings new_settings) {
    nvs_handle_t handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK) return;

    nvs_set_u16(handle, "start_life", new_settings.starting_life);
    nvs_set_u8(handle, "scr_br", new_settings.screen_brightness);
    nvs_set_u8(handle, "loud", new_settings.sound_loudness);
    nvs_set_u8(handle, "dim_t", new_settings.screen_timeout_min);
    nvs_set_u8(handle, "off_t", new_settings.auto_off_min);
    nvs_set_u8(handle, "snd_en", (uint8_t)new_settings.sound_enabled);
    nvs_set_u8(handle, "dead0", (uint8_t)new_settings.dead_at_zero);
    nvs_set_u8(handle, "cmd21", (uint8_t)new_settings.cmd_dmg_rule);

    nvs_commit(handle);
    nvs_close(handle);

    s_current_settings = new_settings;
    GUIRenderer_set_contrast(s_current_settings.screen_brightness);
}

GameSettings SettingsModel_get() { return s_current_settings; }