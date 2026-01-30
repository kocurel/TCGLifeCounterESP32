#include "DeviceSettingsPage.h"

#include <stdio.h>

#include "AudioManager.h"
#include "GUIFramework.h"
#include "GUIRenderer.h"
#include "MenuPage.h"
#include "app/PageManager.h"
#include "model/Settings.h"

// --- Constants & Enums ---
enum {
    OPT_BRIGHTNESS,
    OPT_VOLUME,
    OPT_SCREEN_TIMEOUT,
    OPT_AUTO_OFF,
    OPT_COUNT
};

// --- State ---
static GUIList options_list;
static char s_list_buffer[32];
static GameSettings s_draft_settings;  // Bufor roboczy zmian

// --- Helpers ---

static int settings_get_count(void* data) { return OPT_COUNT; }

static char* settings_item_to_string(void* item, int index) {
    GameSettings* s = &s_draft_settings;

    switch (index) {
        case OPT_BRIGHTNESS: {
            int level = (s->screen_brightness / 32) + 1;
            snprintf(s_list_buffer, sizeof(s_list_buffer), "Brightness: %d",
                     level);
            break;
        }
        case OPT_VOLUME:
            snprintf(s_list_buffer, sizeof(s_list_buffer), "Volume: %d%%",
                     s->sound_loudness);
            break;

        case OPT_SCREEN_TIMEOUT:
            snprintf(s_list_buffer, sizeof(s_list_buffer), "Dim: %d min",
                     s->screen_timeout_min);
            break;

        case OPT_AUTO_OFF:
            snprintf(s_list_buffer, sizeof(s_list_buffer), "Auto Off: %d min",
                     s->auto_off_min);
            break;

        default:
            return "?";
    }
    return s_list_buffer;
}

static void modify_setting(int index, int direction) {
    switch (index) {
        case OPT_BRIGHTNESS:
            // 1. Logika zmiany wartości (to, czego brakowało!)
            if (direction > 0) {
                if (s_draft_settings.screen_brightness >= 128)
                    s_draft_settings.screen_brightness = 0;
                else
                    s_draft_settings.screen_brightness += 32;
            } else {
                if (s_draft_settings.screen_brightness < 32)
                    s_draft_settings.screen_brightness = 128;
                else
                    s_draft_settings.screen_brightness -= 32;
            }
            // Zmieniamy sprzętowo od razu, by użytkownik widział podgląd
            GUIRenderer_set_contrast(s_draft_settings.screen_brightness);
            break;

        case OPT_VOLUME:
            if (direction > 0) {
                if (s_draft_settings.sound_loudness >= 100)
                    s_draft_settings.sound_loudness = 0;
                else
                    s_draft_settings.sound_loudness += 25;
            } else {
                if (s_draft_settings.sound_loudness <= 0)
                    s_draft_settings.sound_loudness = 100;
                else
                    s_draft_settings.sound_loudness -= 25;
            }
            AudioManager_set_volume(s_draft_settings.sound_loudness);
            AudioManager_play_tone(1000, 50);  // Feedback głośności
            break;

        case OPT_SCREEN_TIMEOUT:
            if (direction > 0) {
                if (s_draft_settings.screen_timeout_min == 1)
                    s_draft_settings.screen_timeout_min = 2;
                else if (s_draft_settings.screen_timeout_min == 2)
                    s_draft_settings.screen_timeout_min = 5;
                else if (s_draft_settings.screen_timeout_min == 5)
                    s_draft_settings.screen_timeout_min = 10;
                else
                    s_draft_settings.screen_timeout_min = 1;
            } else {
                if (s_draft_settings.screen_timeout_min == 10)
                    s_draft_settings.screen_timeout_min = 5;
                else if (s_draft_settings.screen_timeout_min == 5)
                    s_draft_settings.screen_timeout_min = 2;
                else if (s_draft_settings.screen_timeout_min == 2)
                    s_draft_settings.screen_timeout_min = 1;
                else
                    s_draft_settings.screen_timeout_min = 10;
            }
            break;

        case OPT_AUTO_OFF:
            if (direction > 0) {
                if (s_draft_settings.auto_off_min == 15)
                    s_draft_settings.auto_off_min = 30;
                else if (s_draft_settings.auto_off_min == 30)
                    s_draft_settings.auto_off_min = 60;
                else
                    s_draft_settings.auto_off_min = 15;
            } else {
                if (s_draft_settings.auto_off_min == 60)
                    s_draft_settings.auto_off_min = 30;
                else if (s_draft_settings.auto_off_min == 30)
                    s_draft_settings.auto_off_min = 15;
                else
                    s_draft_settings.auto_off_min = 60;
            }
            break;
    }
}

// --- Page Logic ---

static void DeviceSettingsPage_update_ui() {
    GUIRenderer_clear_buffer();
    GUI_DRAW(&options_list);

    // Twoja zmiana ramki: y = index * 11 + 3, h = 12
    int visual_index = GUIList_get_current_index(&options_list) % 5;
    GUIRenderer_draw_frame(0, visual_index * 11 + 6, 128, 12);

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
            // ROLLBACK: Przywracamy jasność z NVS przed wyjściem
            GameSettings original = SettingsModel_get();
            GUIRenderer_set_contrast(original.screen_brightness);
            AudioManager_set_volume(original.sound_loudness);
            AudioManager_play_sound(SOUND_UI_CANCEL);
            MenuPage_enter();
            return;
        }

        case BUTTON_CODE_ACCEPT:
            AudioManager_play_sound(SOUND_UI_SELECT);
            // COMMIT: Zapisujemy zmiany na stałe
            SettingsModel_save(s_draft_settings);
            MenuPage_enter();
            return;

        default:
            break;
    }
    DeviceSettingsPage_update_ui();
}

void DeviceSettingsPage_enter() {
    // 1. ZAWSZE odświeżaj draft z aktualnych ustawień przy wejściu
    s_draft_settings = SettingsModel_get();

    // 2. Inicjalizacja komponentów tylko RAZ (oszczędność CPU i stabilność
    // wskaźników)
    static bool initialized = false;
    if (!initialized) {
        GUIList_init(&options_list, NULL, settings_get_count, NULL,
                     settings_item_to_string, NULL);
        GUI_SET_POS(&options_list, 2, 4);
        GUI_SET_SIZE(&options_list, 124, 60);
        initialized = true;
    }

    // 3. Rejestracja strony w managerze
    Page page = {.handle_input = DeviceSettingsPage_handle_input, .exit = NULL};
    PageManager_switch_page(&page);

    // 4. Wymuś odświeżenie grafiki
    DeviceSettingsPage_update_ui();
}