#include "PlayerPage.h"

#include "GUIFramework.h"
#include "MainPage.h"
#include "ValueEditorPage.h"
#include "app/GameDelegates.h"
#include "app/PageManager.h"
#include "model/Game.h"
static GUIList list = {0};
static GUILabel player_lbl = {0};
static bool is_initialized = false;
static int page_player_id = 0;

void PlayerPage_update() {
    GUIRenderer_clear_buffer();
    GUI_DRAW(&list);
    GUI_DRAW(&player_lbl);
    GUIRenderer_draw_horizontal_line(13);
    GUIRenderer_send_buffer();
}
static void PlayerPage_callback(int32_t value);

void PlayerPage_handle_input(ButtonCode button) {
    switch (button) {
        case BUTTON_CODE_DOWN:
            GUIList_down(&list);
            PlayerPage_update();
            break;
        case BUTTON_CODE_UP:
            GUIList_up(&list);
            PlayerPage_update();
            break;
        case BUTTON_CODE_CANCEL:
            MainPage_enter();
            break;
        case BUTTON_CODE_ACCEPT:
            int value_index = GUIList_get_current_index(&list);
            ValueEditorPage_enter(Game_get_player_name(page_player_id),
                                  Game_get_value_name(value_index),
                                  Game_get_value(page_player_id, value_index),
                                  PlayerPage_callback);
            break;
        default:
            break;
    }
}
static void PlayerPage_callback(int32_t value) {
    Game_set_value(value, page_player_id, GUIList_get_current_index(&list));
    Page new_page = {0};
    new_page.handle_input = PlayerPage_handle_input;
    PageManager_switch_page(&new_page);

    PlayerPage_update();
}

void PlayerPage_enter(int player_id) {
    if (!is_initialized) {
        GUIList_init(&list, Game_get_player(player_id),
                     delegate_get_value_count, delegate_get_player_value,
                     delegate_format_player_value, NULL);
        GUI_SET_POS(&list, 0, 14);
        GUI_SET_SIZE(&list, 118, 44);

        GUILabel_init(&player_lbl, Game_get_player(player_id)->name);
        GUI_SET_SIZE(&player_lbl, 128, 12);
        GUI_SET_FONT_SIZE(&player_lbl, 7);

        is_initialized = true;
    }
    page_player_id = player_id;
    Page new_page = {0};
    new_page.handle_input = PlayerPage_handle_input;
    PageManager_switch_page(&new_page);

    PlayerPage_update();
}
