#include "DeviceSettingsPage.h"

#include <math.h>
#include <stdio.h>

#include "AudioManager.h"
#include "GUIFramework.h"
#include "GUIRenderer.h"
#include "SettingsPage.h"
#include "app/PageManager.h"
#include "model/Settings.h"

typedef enum {
    OPT_BRIGHTNESS,
    OPT_VOLUME,
    OPT_QUICK_DMG,
    OPT_SCREEN_TIMEOUT,
    OPT_AUTO_OFF,
    OPT_COUNT
} DeviceOption;

/* Available options for time-based settings in minutes (0 = Never) */
static const uint8_t TIMEOUT_OPTS[] = {0, 1, 2, 5, 10, 30};
static const uint8_t AUTOOFF_OPTS[] = {0, 15, 30, 60, 120};

#define TIMEOUT_OPTS_COUNT (sizeof(TIMEOUT_OPTS) / sizeof(TIMEOUT_OPTS[0]))
#define AUTOOFF_OPTS_COUNT (sizeof(AUTOOFF_OPTS) / sizeof(AUTOOFF_OPTS[0]))

static GUIList options_list;
static char s_list_buffer[32];
static GameSettings s_draft_settings;

/**
 * Iterates through a circular array of configuration options
 */
static uint8_t cycle_value(uint8_t current, const uint8_t* array, int size,
                           int direction) {
    int current_idx = 0;
    for (int i = 0; i < size; i++) {
        if (array[i] == current) {
            current_idx = i;
            break;
        }
    }
    if (direction > 0) {
        current_idx = (current_idx + 1) % size;
    } else {
        current_idx = (current_idx - 1 + size) % size;
    }
    return array[current_idx];
}

static int settings_get_count(void* data) { return OPT_COUNT; }

/**
 * Maps device-specific settings to formatted display strings
 */
static char* settings_item_to_string(void* item, int index) {
    GameSettings* s = &s_draft_settings;
    switch (index) {
        case OPT_BRIGHTNESS: {
            int level = (s->screen_brightness > 0)
                            ? (s->screen_brightness / 32) + 1
                            : 1;
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
            if (s->screen_timeout_min == 0)
                snprintf(s_list_buffer, sizeof(s_list_buffer), "Dim: Never");
            else
                snprintf(s_list_buffer, sizeof(s_list_buffer), "Dim: %d min",
                         s->screen_timeout_min);
            break;
        case OPT_AUTO_OFF:
            if (s->auto_off_min == 0)
                snprintf(s_list_buffer, sizeof(s_list_buffer),
                         "Auto Off: Never");
            else
                snprintf(s_list_buffer, sizeof(s_list_buffer),
                         "Auto Off: %d min", s->auto_off_min);
            break;
        default:
            return "?";
    }
    return s_list_buffer;
}

/**
 * Adjusts session settings and provides immediate hardware feedback
 */
static void modify_setting(int index, int direction) {
    switch (index) {
        case OPT_BRIGHTNESS:
            if (direction > 0) {
                if (s_draft_settings.screen_brightness < 128)
                    s_draft_settings.screen_brightness += 32;
            } else {
                if (s_draft_settings.screen_brightness >= 32)
                    s_draft_settings.screen_brightness -= 32;
            }
            GUIRenderer_set_contrast(s_draft_settings.screen_brightness);
            break;

        case OPT_VOLUME:
            if (direction > 0) {
                if (s_draft_settings.sound_loudness < 100)
                    s_draft_settings.sound_loudness += 25;
                else
                    s_draft_settings.sound_loudness = 0;
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

    options_list.needs_redraw = true;
}

static void DeviceSettingsPage_draw() {
    GUIRenderer_clear_buffer();
    GUI_DRAW(&options_list);
    GUIRenderer_send_buffer();
}

/**
 * Updates list animation and triggers redraw if cursor position changes
 */
static void DeviceSettingsPage_on_tick(uint32_t delta_ms) {
    const float old_y = options_list.anim_y;
    GUIList_tick(&options_list, delta_ms);

    if (fabsf(options_list.anim_y - old_y) > 0.05f) {
        options_list.needs_redraw = true;
    }

    if (options_list.needs_redraw) {
        DeviceSettingsPage_draw();
        options_list.needs_redraw = false;
    }
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
            /* Restore persistent hardware settings on cancel */
            const GameSettings original = SettingsModel_get();
            GUIRenderer_set_contrast(original.screen_brightness);
            AudioManager_set_volume(original.sound_loudness);
            AudioManager_play_sound(SOUND_UI_CANCEL);
            SettingsPage_enter();
            return;
        }

        case BUTTON_CODE_ACCEPT:
            /* Persist session settings to storage */
            AudioManager_play_sound(SOUND_UI_SELECT);
            SettingsModel_save(s_draft_settings);
            SettingsPage_enter();
            return;
        default:
            break;
    }
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

    /* Snap animation targets on entry to prevent artifacts */
    const int visible_rows = options_list.base.height / 11;
    const int relative_row = options_list.selected_index % visible_rows;
    options_list.anim_y = (float)(options_list.base.y + (relative_row * 11));
    options_list.needs_redraw = false;

    Page page = {.handle_input = DeviceSettingsPage_handle_input,
                 .on_tick = DeviceSettingsPage_on_tick};
    PageManager_switch_page(&page);
    DeviceSettingsPage_draw();
}