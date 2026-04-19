#include "PlayerSettingsPage.h"

#include <math.h>
#include <stdio.h>

#include "GUIFramework.h"
#include "KeyboardPage.h"
#include "PlayerPage.h"
#include "app/PageManager.h"
#include "model/Game.h"
#include "model/Settings.h"

typedef enum {
    OPT_EDIT_NAME,
    OPT_MONARCH,
    OPT_CITY_BLESSING,
    OPT_COUNT
} SettingOption;

static GUIList settings_list;
static GUILabel title;
static int active_player_id;

/**
 * Returns total count of available player settings
 */
static int player_settings_get_count(void* data) { return OPT_COUNT; }

/**
 * Formats player settings labels with current state values
 */
static char* player_opt_to_str(void* item, int index) {
    static char buf[32];
    Player* p = Game_get_player(active_player_id);

    switch (index) {
        case OPT_EDIT_NAME:
            snprintf(buf, sizeof(buf), "Rename: %s", p->name);
            break;
        case OPT_MONARCH:
            snprintf(buf, sizeof(buf), "Monarch: %s",
                     Game_is_monarch(active_player_id) ? "YES" : "No");
            break;
        case OPT_CITY_BLESSING: {
            int32_t blessing =
                Game_get_value(active_player_id, INDEX_CITY_BLESSING);
            snprintf(buf, sizeof(buf), "Blessing: %s", blessing ? "ON" : "Off");
            break;
        }
        default:
            return "";
    }
    return buf;
}

static void PlayerSettingsPage_draw() {
    GUIRenderer_clear_buffer();

    GUI_DRAW(&title);
    GUIRenderer_draw_horizontal_line(13);
    GUI_DRAW(&settings_list);

    GUIRenderer_send_buffer();
}

/**
 * Updates list animation and triggers redraw on movement or state change
 */
static void PlayerSettingsPage_on_tick(uint32_t delta_ms) {
    const float old_y = settings_list.anim_y;
    GUIList_tick(&settings_list, delta_ms);

    if (fabsf(settings_list.anim_y - old_y) > 0.05f) {
        settings_list.needs_redraw = true;
    }

    if (settings_list.needs_redraw) {
        PlayerSettingsPage_draw();
        settings_list.needs_redraw = false;
    }
}

/**
 * Persists updated name and re-enters page to refresh view
 */
static void on_name_complete(const char* new_name) {
    if (new_name) {
        Game_set_player_name(active_player_id, new_name);
        SettingsModel_save_player_name(active_player_id, new_name);
    }
    PlayerSettingsPage_enter(active_player_id);
}

static void PlayerPage_handle_input(ButtonCode btn) {
    const int idx = GUIList_get_current_index(&settings_list);

    switch (btn) {
        case BUTTON_CODE_UP:
            GUIList_up(&settings_list);
            break;
        case BUTTON_CODE_DOWN:
            GUIList_down(&settings_list);
            break;
        case BUTTON_CODE_ACCEPT:
            if (idx == OPT_EDIT_NAME) {
                Player* p = Game_get_player(active_player_id);
                KeyboardPage_enter("ENTER NAME", p->name, on_name_complete);
                return;
            } else if (idx == OPT_MONARCH) {
                Game_set_monarch(active_player_id);
                settings_list.needs_redraw = true;
            } else if (idx == OPT_CITY_BLESSING) {
                Game_toggle_blessing(active_player_id);
                settings_list.needs_redraw = true;
            }
            break;
        case BUTTON_CODE_CANCEL:
            PlayerPage_enter(active_player_id);
            return;
        default:
            break;
    }
}

void PlayerSettingsPage_enter(int player_id) {
    active_player_id = player_id;

    static bool initialized = false;
    if (!initialized) {
        GUILabel_init(&title, "PLAYER SETTINGS");
        GUI_SET_FONT_SIZE(&title, 7);
        GUI_SET_SIZE(&title, 128, 12);
        GUILabel_set_alignment(&title, GUI_ALIGMNENT_CENTER);

        GUIList_init(&settings_list, NULL, player_settings_get_count, NULL,
                     player_opt_to_str, NULL);
        GUI_SET_POS(&settings_list, 0, 14);
        GUI_SET_SIZE(&settings_list, 124, 45);

        initialized = true;
    }

    /* Immediate animation snap on entry */
    const int visible_rows = settings_list.base.height / 11;
    const int relative_row = settings_list.selected_index % visible_rows;
    settings_list.anim_y = (float)(settings_list.base.y + (relative_row * 11));
    settings_list.needs_redraw = false;

    Page page = {.handle_input = PlayerPage_handle_input,
                 .on_tick = PlayerSettingsPage_on_tick};

    PageManager_switch_page(&page);
    PlayerSettingsPage_draw();
}