#include "GameSettingsPage.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "AudioManager.h"
#include "GUIFramework.h"
#include "GUIRenderer.h"
#include "SettingsPage.h"
#include "app/PageManager.h"
#include "model/Settings.h"

typedef enum {
    OPT_START_LIFE,
    OPT_PLAYER_COUNT,
    OPT_DEAD_AT_ZERO,
    OPT_CMD_EN,
    OPT_CMD_DMG_RULE,
    OPT_COUNT,
} SettingOption;

static GUIList options_list;
static char s_list_buffer[32];
static GameSettings s_draft_settings;

/**
 * Returns total number of available game settings
 */
static int settings_get_count(void* data) { return OPT_COUNT; }

/**
 * Maps game rules and settings to displayable strings
 */
static char* settings_item_to_string(void* item, int index) {
    GameSettings* s = &s_draft_settings;

    switch (index) {
        case OPT_START_LIFE:
            snprintf(s_list_buffer, sizeof(s_list_buffer), "Start Life: %d",
                     s->starting_life);
            break;

        case OPT_PLAYER_COUNT:
            snprintf(s_list_buffer, sizeof(s_list_buffer), "Players: %d",
                     s->player_count);
            break;

        case OPT_DEAD_AT_ZERO:
            snprintf(s_list_buffer, sizeof(s_list_buffer), "Dead at 0: %s",
                     s->dead_at_zero ? "Yes" : "No");
            break;

        case OPT_CMD_EN:
            snprintf(s_list_buffer, sizeof(s_list_buffer), "Cmd enabled: %s",
                     s->cmd_mode_en ? "Yes" : "No");
            break;

        case OPT_CMD_DMG_RULE:
            snprintf(s_list_buffer, sizeof(s_list_buffer), "Cmd Rule 21: %s",
                     s->cmd_dmg_rule ? "On" : "Off");
            break;

        default:
            return "?";
    }
    return s_list_buffer;
}

/**
 * Cycles through discrete game setting values based on input direction
 */
static void modify_setting(int index, int direction) {
    AudioManager_play_sound(SOUND_UI_MOVE);

    switch (index) {
        case OPT_START_LIFE: {
            const uint16_t values[] = {20, 25, 30, 40, 50, 4000, 8000};
            const int count = sizeof(values) / sizeof(values[0]);

            int current_idx = 0;
            bool found = false;
            for (int i = 0; i < count; i++) {
                if (s_draft_settings.starting_life == values[i]) {
                    current_idx = i;
                    found = true;
                    break;
                }
            }
            if (!found) current_idx = 3;

            if (direction > 0) {
                current_idx = (current_idx + 1) % count;
            } else {
                current_idx = (current_idx - 1 + count) % count;
            }
            s_draft_settings.starting_life = values[current_idx];
            break;
        }

        case OPT_PLAYER_COUNT:
            s_draft_settings.player_count =
                (s_draft_settings.player_count == 2) ? 4 : 2;
            break;

        case OPT_DEAD_AT_ZERO:
            s_draft_settings.dead_at_zero = !s_draft_settings.dead_at_zero;
            break;

        case OPT_CMD_DMG_RULE:
            s_draft_settings.cmd_dmg_rule = !s_draft_settings.cmd_dmg_rule;
            break;

        case OPT_CMD_EN:
            s_draft_settings.cmd_mode_en = !s_draft_settings.cmd_mode_en;
            break;
    }

    /* Request redraw to reflect updated setting value in list */
    options_list.needs_redraw = true;
}

static void GameSettingsPage_draw() {
    GUIRenderer_clear_buffer();
    GUI_DRAW(&options_list);
    GUIRenderer_send_buffer();
}

/**
 * Updates list animation and triggers redraw on significant cursor movement
 */
static void GameSettingsPage_on_tick(uint32_t delta_ms) {
    const float old_y = options_list.anim_y;
    GUIList_tick(&options_list, delta_ms);

    if (fabsf(options_list.anim_y - old_y) > 0.05f) {
        options_list.needs_redraw = true;
    }

    if (options_list.needs_redraw) {
        GameSettingsPage_draw();
        options_list.needs_redraw = false;
    }
}

static void GameSettingsPage_handle_input(ButtonCode button) {
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
        case BUTTON_CODE_CANCEL:
            AudioManager_play_sound(SOUND_UI_CANCEL);
            SettingsPage_enter();
            return;
        case BUTTON_CODE_ACCEPT:
            /* Persist draft settings to storage */
            AudioManager_play_sound(SOUND_UI_SELECT);
            SettingsModel_save(s_draft_settings);
            SettingsPage_enter();
            return;
        default:
            break;
    }
}

void GameSettingsPage_enter() {
    /* Initialize session state with persistent settings */
    s_draft_settings = SettingsModel_get();

    static bool initialized = false;
    if (!initialized) {
        GUIList_init(&options_list, NULL, settings_get_count, NULL,
                     settings_item_to_string, NULL);
        GUI_SET_POS(&options_list, 2, 4);
        GUI_SET_SIZE(&options_list, 124, 60);
        initialized = true;
    }

    /* Snap animation to avoid artifacts on entry */
    const int visible_rows = options_list.base.height / 11;
    const int relative_row = options_list.selected_index % visible_rows;
    options_list.anim_y = (float)(options_list.base.y + (relative_row * 11));
    options_list.needs_redraw = false;

    Page page = {.handle_input = GameSettingsPage_handle_input,
                 .on_tick = GameSettingsPage_on_tick};

    PageManager_switch_page(&page);
    GameSettingsPage_draw();
}