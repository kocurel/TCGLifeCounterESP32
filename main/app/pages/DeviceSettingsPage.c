#include "DeviceSettingsPage.h"

#include <stdio.h>

#include "AudioManager.h"
#include "GUIFramework.h"
#include "GUIRenderer.h"
#include "SettingsPage.h"
#include "app/PageManager.h"
#include "model/Settings.h"

// --- Constants & Enums ---
enum {
    OPT_BRIGHTNESS,
    OPT_VOLUME,
    OPT_QUICK_DMG,
    OPT_SCREEN_TIMEOUT,
    OPT_AUTO_OFF,
    OPT_COUNT
};

// --- Dostępne wartości (w minutach, 0 = Nigdy) ---
static const uint8_t TIMEOUT_OPTS[] = {0, 1, 2, 5, 10, 30};
static const uint8_t AUTOOFF_OPTS[] = {0, 15, 30, 60, 120};

#define TIMEOUT_OPTS_COUNT (sizeof(TIMEOUT_OPTS) / sizeof(TIMEOUT_OPTS[0]))
#define AUTOOFF_OPTS_COUNT (sizeof(AUTOOFF_OPTS) / sizeof(AUTOOFF_OPTS[0]))

// --- State ---
static GUIList options_list;
static char s_list_buffer[32];
static GameSettings s_draft_settings;

// --- Helper Logic ---

// Uniwersalna funkcja do cyklicznego przechodzenia po tablicy wartości
static uint8_t cycle_value(uint8_t current, const uint8_t* array, int size,
                           int direction) {
    int current_idx = 0;
    // 1. Znajdź obecny indeks
    for (int i = 0; i < size; i++) {
        if (array[i] == current) {
            current_idx = i;
            break;
        }
    }

    // 2. Przesuń indeks
    if (direction > 0) {
        current_idx++;
        if (current_idx >= size) current_idx = 0;  // Wrap to start
    } else {
        current_idx--;
        if (current_idx < 0) current_idx = size - 1;  // Wrap to end
    }

    return array[current_idx];
}

static int settings_get_count(void* data) { return OPT_COUNT; }

static char* settings_item_to_string(void* item, int index) {
    GameSettings* s = &s_draft_settings;

    switch (index) {
        case OPT_BRIGHTNESS: {
            // Brightness 0-128 mapujemy na poziom 1-5 dla czytelności
            int level = (s->screen_brightness > 0)
                            ? (s->screen_brightness / 32) + 1
                            : 0;
            // Zabezpieczenie wizualne (żeby nie pokazywać "Brightness: 0" jeśli
            // jasność minimalna to np 1)
            if (level == 0) level = 1;
            snprintf(s_list_buffer, sizeof(s_list_buffer), "Brightness: %d",
                     level);
            break;
        }
        case OPT_VOLUME:
            snprintf(s_list_buffer, sizeof(s_list_buffer), "Volume: %d%%",
                     s->sound_loudness);
            break;

        case OPT_QUICK_DMG:
            snprintf(s_list_buffer, sizeof(s_list_buffer), "Quick Life: %s",
                     s->quick_dmg_en ? "ON" : "OFF");
            break;

        case OPT_SCREEN_TIMEOUT:
            if (s->screen_timeout_min == 0) {
                snprintf(s_list_buffer, sizeof(s_list_buffer), "Dim: Never");
            } else {
                snprintf(s_list_buffer, sizeof(s_list_buffer), "Dim: %d min",
                         s->screen_timeout_min);
            }
            break;

        case OPT_AUTO_OFF:
            if (s->auto_off_min == 0) {
                snprintf(s_list_buffer, sizeof(s_list_buffer),
                         "Auto Off: Never");
            } else {
                snprintf(s_list_buffer, sizeof(s_list_buffer),
                         "Auto Off: %d min", s->auto_off_min);
            }
            break;

        default:
            return "?";
    }
    return s_list_buffer;
}

static void modify_setting(int index, int direction) {
    switch (index) {
        case OPT_BRIGHTNESS:
            // Logika liniowa (nie zapętla się, bo ciężko trafić w ciemnościach
            // jak przeskoczy na 0)
            if (direction > 0) {
                if (s_draft_settings.screen_brightness < 128)
                    s_draft_settings.screen_brightness += 32;
            } else {
                if (s_draft_settings.screen_brightness > 32)
                    s_draft_settings.screen_brightness -= 32;
            }
            GUIRenderer_set_contrast(s_draft_settings.screen_brightness);
            break;

        case OPT_VOLUME:
            if (direction > 0) {
                if (s_draft_settings.sound_loudness < 100)
                    s_draft_settings.sound_loudness += 25;
                else
                    s_draft_settings.sound_loudness = 0;  // Tu zapętlamy
            } else {
                if (s_draft_settings.sound_loudness > 0)
                    s_draft_settings.sound_loudness -= 25;
                else
                    s_draft_settings.sound_loudness = 100;
            }
            AudioManager_set_volume(s_draft_settings.sound_loudness);
            AudioManager_play_tone(1000, 50);
            break;

        case OPT_QUICK_DMG:
            s_draft_settings.quick_dmg_en = !s_draft_settings.quick_dmg_en;
            AudioManager_play_sound(SOUND_UI_SELECT);
            break;

        case OPT_SCREEN_TIMEOUT:
            s_draft_settings.screen_timeout_min =
                cycle_value(s_draft_settings.screen_timeout_min, TIMEOUT_OPTS,
                            TIMEOUT_OPTS_COUNT, direction);
            break;

        case OPT_AUTO_OFF:
            s_draft_settings.auto_off_min =
                cycle_value(s_draft_settings.auto_off_min, AUTOOFF_OPTS,
                            AUTOOFF_OPTS_COUNT, direction);
            break;
    }
}

static void DeviceSettingsPage_draw() {
    GUIRenderer_clear_buffer();
    GUI_DRAW(&options_list);

    int visual_index = GUIList_get_current_index(&options_list);
    GUIRenderer_draw_frame(0, visual_index * 11 + 4, 128, 12);

    GUIRenderer_send_buffer();
}

static void DeviceSettingsPage_handle_input(ButtonCode button) {
    switch (button) {
        case BUTTON_CODE_UP:
            GUIList_up(&options_list);
            break;
        case BUTTON_CODE_DOWN:
            GUIList_down(&options_list);
            break;
        case BUTTON_CODE_RIGHT:
            modify_setting(GUIList_get_current_index(&options_list), 1);
            break;
        case BUTTON_CODE_LEFT:
            modify_setting(GUIList_get_current_index(&options_list), -1);
            break;

        case BUTTON_CODE_CANCEL: {
            // Przywróć oryginalne ustawienia hardware'owe (jasność/głośność)
            // jeśli anulowano
            GameSettings original = SettingsModel_get();
            GUIRenderer_set_contrast(original.screen_brightness);
            AudioManager_set_volume(original.sound_loudness);
            AudioManager_play_sound(SOUND_UI_CANCEL);
            SettingsPage_enter();
            return;
        }

        case BUTTON_CODE_ACCEPT:
            AudioManager_play_sound(SOUND_UI_SELECT);
            SettingsModel_save(s_draft_settings);
            SettingsPage_enter();
            return;

        default:
            break;
    }
    DeviceSettingsPage_draw();
}

void DeviceSettingsPage_enter() {
    s_draft_settings = SettingsModel_get();

    static bool initialized = false;
    if (!initialized) {
        GUIList_init(&options_list, NULL, settings_get_count, NULL,
                     settings_item_to_string, NULL);
        GUI_SET_POS(&options_list, 2, 2);
        GUI_SET_SIZE(&options_list, 124, 56);
        initialized = true;
    }

    Page page = {.handle_input = DeviceSettingsPage_handle_input, .exit = NULL};
    PageManager_switch_page(&page);
    DeviceSettingsPage_draw();
}