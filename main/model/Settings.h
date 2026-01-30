#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdbool.h>
#include <stdint.h>

#define BRIGHTNESS_MIN 0
#define BRIGHTNESS_MAX 128
#define BRIGHTNESS_STEP 32

typedef struct {
    // --- Device (Sprzęt) ---
    uint8_t screen_brightness;
    uint8_t sound_loudness;
    uint8_t screen_timeout_min;
    uint8_t auto_off_min;
    bool sound_enabled;

    // --- Game Rules (Zasady) ---
    uint16_t starting_life;  // Obsługuje MTG (40) i YGO (8000)
    bool dead_at_zero;       // Czy 0 HP = przegrana
    bool cmd_dmg_rule;       // Czy 21 Commander Damage = przegrana
} GameSettings;

void SettingsModel_init();
GameSettings SettingsModel_get();
void SettingsModel_save(GameSettings new_settings);

#endif