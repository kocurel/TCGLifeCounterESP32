#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdbool.h>
#include <stdint.h>

#define BRIGHTNESS_MIN 0
#define BRIGHTNESS_MAX 128
#define BRIGHTNESS_STEP 32
#define PLAYER_NAME_MAX_LEN 16

typedef struct {
    // --- Device (Sprzęt) ---
    uint8_t screen_brightness;
    uint8_t sound_loudness;
    uint8_t screen_timeout_min;
    uint8_t auto_off_min;
    bool sound_enabled;
    bool quick_dmg_en;  // <--- NOWE: Tryb szybkiego zadawania obrażeń

    // --- Game Rules (Zasady) ---
    uint16_t starting_life;
    uint8_t player_count;
    bool dead_at_zero;
    bool cmd_dmg_rule;
    bool cmd_mode_en;
    // --- Persistent Player Names ---
    char player_names[4][PLAYER_NAME_MAX_LEN];
} GameSettings;

void SettingsModel_init();
GameSettings SettingsModel_get();
void SettingsModel_save(GameSettings new_settings);
void SettingsModel_save_player_name(int player_id, const char* name);

#endif