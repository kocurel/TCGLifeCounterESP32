#include "Settings.h"

#include <stdio.h>
#include <string.h>

#include "Debug.h"
#include "GUIRenderer.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char* NVS_NAMESPACE = "game_config";

static GameSettings s_current_settings;

// Helper to set defaults for value names
// In Settings.c

static void set_default_value_names(GameSettings* settings) {
    // 1. Initialize ALL to generic "Val X" first
    for (int i = 0; i < SETTINGS_VAL_COUNT; i++) {
        snprintf(settings->value_names[i], SETTINGS_VAL_NAME_LEN, "Val %d",
                 i + 1);
    }

    // 2. Set Specific Defaults
    strncpy(settings->value_names[0], "HP", SETTINGS_VAL_NAME_LEN);
    strncpy(settings->value_names[1], "Poison", SETTINGS_VAL_NAME_LEN);
    strncpy(settings->value_names[2], "Energy", SETTINGS_VAL_NAME_LEN);
    strncpy(settings->value_names[3], "Exp", SETTINGS_VAL_NAME_LEN);

    strncpy(settings->value_names[8], "Monarch", SETTINGS_VAL_NAME_LEN);
    strncpy(settings->value_names[9], "City Bls", SETTINGS_VAL_NAME_LEN);

    // --- CHANGED: Default names for indices 28-31 ---
    strncpy(settings->value_names[28], "Cmd Dmg 1", SETTINGS_VAL_NAME_LEN);
    strncpy(settings->value_names[29], "Cmd Dmg 2", SETTINGS_VAL_NAME_LEN);
    strncpy(settings->value_names[30], "Cmd Dmg 3", SETTINGS_VAL_NAME_LEN);
    strncpy(settings->value_names[31], "Cmd Dmg 4", SETTINGS_VAL_NAME_LEN);
}

static const GameSettings DEFAULT_SETTINGS = {
    .starting_life = 40,
    .player_count = 4,
    .screen_brightness = 32,
    .sound_loudness = 50,
    .screen_timeout_min = 2,
    .auto_off_min = 15,
    .sound_enabled = true,
    .quick_dmg_en = false,
    .dead_at_zero = true,
    .cmd_dmg_rule = true,
    .cmd_mode_en = false,
    .player_names = {"Player 1", "Player 2", "Player 3", "Player 4"}
    // value_names are initialized dynamically below
};
GameSettings SettingsModel_get() { return s_current_settings; }

void SettingsModel_init() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    // Initialize defaults first
    s_current_settings = DEFAULT_SETTINGS;
    set_default_value_names(&s_current_settings);

    nvs_handle_t handle;
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return;  // Return with defaults
    }

    uint8_t val8;
    uint16_t val16;

    // --- Loading Numeric Settings ---
    if (nvs_get_u16(handle, "start_life", &val16) == ESP_OK)
        s_current_settings.starting_life = val16;

    if (nvs_get_u8(handle, "p_cnt", &val8) == ESP_OK) {
        if (val8 == 2 || val8 == 4) s_current_settings.player_count = val8;
    }

    if (nvs_get_u8(handle, "scr_br", &val8) == ESP_OK)
        s_current_settings.screen_brightness = val8;

    if (nvs_get_u8(handle, "loud", &val8) == ESP_OK)
        s_current_settings.sound_loudness = val8;

    if (nvs_get_u8(handle, "dim_t", &val8) == ESP_OK)
        s_current_settings.screen_timeout_min = val8;

    if (nvs_get_u8(handle, "off_t", &val8) == ESP_OK)
        s_current_settings.auto_off_min = val8;

    if (nvs_get_u8(handle, "snd_en", &val8) == ESP_OK)
        s_current_settings.sound_enabled = (bool)val8;

    if (nvs_get_u8(handle, "q_dmg", &val8) == ESP_OK)
        s_current_settings.quick_dmg_en = (bool)val8;

    if (nvs_get_u8(handle, "dead0", &val8) == ESP_OK)
        s_current_settings.dead_at_zero = (bool)val8;

    if (nvs_get_u8(handle, "cmd21", &val8) == ESP_OK)
        s_current_settings.cmd_dmg_rule = (bool)val8;

    if (nvs_get_u8(handle, "cmd_en", &val8) == ESP_OK)
        s_current_settings.cmd_mode_en = (bool)val8;

    // --- Loading Player Names ---
    char key_buf[8];
    for (int i = 0; i < 4; i++) {
        snprintf(key_buf, sizeof(key_buf), "pn_%d", i);
        size_t required_size = PLAYER_NAME_MAX_LEN;
        nvs_get_str(handle, key_buf, s_current_settings.player_names[i],
                    &required_size);
    }

    // --- [NEW] Loading Value Names ---
    // Keys: vn_0 to vn_31
    for (int i = 0; i < SETTINGS_VAL_COUNT; i++) {
        snprintf(key_buf, sizeof(key_buf), "vn_%d", i);
        size_t required_size = SETTINGS_VAL_NAME_LEN;
        // If key exists, overwrite default. If not, keep default.
        nvs_get_str(handle, key_buf, s_current_settings.value_names[i],
                    &required_size);
    }

    nvs_close(handle);
    GUIRenderer_set_contrast(s_current_settings.screen_brightness);
}

void SettingsModel_save(GameSettings new_settings) {
    nvs_handle_t handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK) return;

    // Save Basic Types
    nvs_set_u16(handle, "start_life", new_settings.starting_life);
    nvs_set_u8(handle, "p_cnt", new_settings.player_count);
    nvs_set_u8(handle, "scr_br", new_settings.screen_brightness);
    nvs_set_u8(handle, "loud", new_settings.sound_loudness);
    nvs_set_u8(handle, "dim_t", new_settings.screen_timeout_min);
    nvs_set_u8(handle, "off_t", new_settings.auto_off_min);
    nvs_set_u8(handle, "snd_en", (uint8_t)new_settings.sound_enabled);
    nvs_set_u8(handle, "q_dmg", (uint8_t)new_settings.quick_dmg_en);
    nvs_set_u8(handle, "dead0", (uint8_t)new_settings.dead_at_zero);
    nvs_set_u8(handle, "cmd21", (uint8_t)new_settings.cmd_dmg_rule);
    nvs_set_u8(handle, "cmd_en", (uint8_t)new_settings.cmd_mode_en);

    char key_buf[8];
    // Save Player Names
    for (int i = 0; i < 4; i++) {
        snprintf(key_buf, sizeof(key_buf), "pn_%d", i);
        nvs_set_str(handle, key_buf, new_settings.player_names[i]);
    }

    // [NEW] Save Value Names
    for (int i = 0; i < SETTINGS_VAL_COUNT; i++) {
        snprintf(key_buf, sizeof(key_buf), "vn_%d", i);
        nvs_set_str(handle, key_buf, new_settings.value_names[i]);
    }

    nvs_commit(handle);
    nvs_close(handle);

    s_current_settings = new_settings;
    GUIRenderer_set_contrast(s_current_settings.screen_brightness);
}

void SettingsModel_save_player_name(int player_id, const char* name) {
    if (player_id < 0 || player_id > 3) return;

    // Update RAM
    strncpy(s_current_settings.player_names[player_id], name,
            PLAYER_NAME_MAX_LEN);
    s_current_settings.player_names[player_id][PLAYER_NAME_MAX_LEN - 1] = '\0';

    // Update NVS
    nvs_handle_t handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) == ESP_OK) {
        char key_buf[8];
        snprintf(key_buf, sizeof(key_buf), "pn_%d", player_id);
        nvs_set_str(handle, key_buf,
                    s_current_settings.player_names[player_id]);
        nvs_commit(handle);
        nvs_close(handle);
    }
}

// [NEW] Implementation for saving a single value name
void SettingsModel_save_value_name(int value_index, const char* name) {
    if (value_index < 0 || value_index >= SETTINGS_VAL_COUNT) return;

    // 1. Update RAM model
    strncpy(s_current_settings.value_names[value_index], name,
            SETTINGS_VAL_NAME_LEN);
    // Ensure null termination
    s_current_settings.value_names[value_index][SETTINGS_VAL_NAME_LEN - 1] =
        '\0';

    // 2. Update NVS (Flash)
    nvs_handle_t handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) == ESP_OK) {
        char key_buf[8];
        // Create key like "vn_0", "vn_1", etc.
        snprintf(key_buf, sizeof(key_buf), "vn_%d", value_index);

        nvs_set_str(handle, key_buf,
                    s_current_settings.value_names[value_index]);
        nvs_commit(handle);
        nvs_close(handle);

        LOG_DEBUG("SettingsModel", "Saved value name [%d]: %s", value_index,
                  name);
    } else {
        LOG_DEBUG("SettingsModel", "Failed to open NVS for saving value name");
    }
}