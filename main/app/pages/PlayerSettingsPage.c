#include "PlayerSettingsPage.h"

#include <stdio.h>

#include "GUIFramework.h"
#include "KeyboardPage.h"
#include "PlayerPage.h"
#include "app/PageManager.h"
#include "model/Game.h"
#include "model/Settings.h"  // Dodane dla zapisu ustawień

enum { OPT_EDIT_NAME, OPT_MONARCH, OPT_CITY_BLESSING, OPT_COUNT };

static GUIList settings_list;
static GUILabel title;
static int active_player_id;

static int player_settings_get_count(void* data) { return OPT_COUNT; }

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
        case OPT_CITY_BLESSING:
            int32_t blessing =
                Game_get_value(active_player_id, INDEX_CITY_BLESSING);
            snprintf(buf, sizeof(buf), "Blessing: %s", blessing ? "ON" : "Off");
            break;
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

static void on_name_complete(const char* new_name) {
    if (new_name) {
        // 1. Zapisz w bieżącej grze (RAM)
        Game_set_player_name(active_player_id, new_name);

        // 2. Zapisz trwale w NVS (Flash)
        SettingsModel_save_player_name(active_player_id, new_name);
    }
    PlayerSettingsPage_enter(active_player_id);
}

static void handle_input(ButtonCode btn) {
    int idx = GUIList_get_current_index(&settings_list);

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
            } else if (idx == OPT_CITY_BLESSING) {
                Game_toggle_blessing(active_player_id);
            }
            break;
        case BUTTON_CODE_CANCEL:
            PlayerPage_enter(active_player_id);
            return;
        default:
            break;
    }
    PlayerSettingsPage_draw();
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
    Page page = {.handle_input = handle_input, .exit = NULL};
    PageManager_switch_page(&page);
    PlayerSettingsPage_draw();
}