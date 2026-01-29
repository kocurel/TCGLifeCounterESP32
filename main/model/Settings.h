#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdbool.h>
#include <stdint.h>

// Struktura trzymająca wszystkie ustawienia gry
typedef struct {
    uint8_t starting_life;          // Np. 20, 40
    uint8_t starting_player_count;  // Np. 2, 4
    uint8_t screen_brightness;      // 0-100
    bool sound_enabled;             // true/false
} GameSettings;

// Inicjalizacja (ładuje z NVS lub ustawia domyślne)
void SettingsModel_init();

// Pobieranie kopii ustawień (z RAM)
GameSettings SettingsModel_get();

// Zapisywanie ustawień (aktualizuje RAM i NVS)
void SettingsModel_save(GameSettings new_settings);

#endif  // SETTINGS_H