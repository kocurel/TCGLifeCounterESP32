#include "PlayerPage.h"

#include "GUIFramework.h"
#include "MainPage.h"
#include "PlayerSettingsPage.h"
#include "ValueEditorPage.h"
#include "app/GameDelegates.h"
#include "app/PageManager.h"
#include "model/Game.h"

/* --- Private State --- */
static GUIList list = {0};
static GUILabel player_lbl = {0};
static int page_player_id = 0;

/* --- Drawing --- */

static void PlayerPage_draw() {
    GUIRenderer_clear_buffer();

    // 1. Render standard components
    GUI_DRAW(&list);
    GUI_DRAW(&player_lbl);
    GUIRenderer_draw_horizontal_line(13);

    // 2. Pagination Indicators
    const int ROW_HEIGHT = 11;
    int visible_rows = list.base.height / ROW_HEIGHT;
    int current_idx = GUIList_get_current_index(&list);
    int total = NUMBER_OF_VALUES;

    // Draw UP indicator if list is scrolled down
    if (current_idx >= visible_rows) {
        GUIRenderer_draw_pixel(64, list.base.y + 1);
    }

    // Draw DOWN indicator if there are more items below
    if (current_idx < total - visible_rows) {
        GUIRenderer_draw_pixel(64, list.base.y + list.base.height + 2);
    }

    GUIRenderer_send_buffer();
}

static void PlayerPage_handle_input(ButtonCode button);
/* --- Callbacks & Handlers --- */

/**
 * Returns from the Value Editor and commits the new value to the model
 */
static void PlayerPage_callback(int32_t value) {
    Game_set_value(value, page_player_id, GUIList_get_current_index(&list));

    // Restore page state
    Page new_page = {.handle_input = PlayerPage_handle_input, .exit = NULL};
    PageManager_switch_page(&new_page);
    PlayerPage_draw();
}

static void PlayerPage_handle_input(ButtonCode button) {
    switch (button) {
        case BUTTON_CODE_UP:
            GUIList_up(&list);
            break;
        case BUTTON_CODE_DOWN:
            GUIList_down(&list);
            break;

        case BUTTON_CODE_LEFT:
            // Fast scroll up
            for (int i = 0; i < 4; i++) GUIList_up(&list);
            break;
        case BUTTON_CODE_RIGHT:
            // Fast scroll down
            for (int i = 0; i < 4; i++) GUIList_down(&list);
            break;

        case BUTTON_CODE_CANCEL:
            MainPage_enter();
            return;

        case BUTTON_CODE_ACCEPT: {
            int value_index = GUIList_get_current_index(&list);
            ValueEditorPage_enter(Game_get_player_name(page_player_id),
                                  Game_get_value_name(value_index), value_index,
                                  Game_get_value(page_player_id, value_index),
                                  PlayerPage_callback);
            return;
        }

        case BUTTON_CODE_MENU:
            // Enter secondary player settings (Name, Monarch status, etc.)
            PlayerSettingsPage_enter(page_player_id);
            return;

        default:
            break;
    }
    PlayerPage_draw();
}

/* --- Page Lifecycle --- */

void PlayerPage_enter(int player_id) {
    page_player_id = player_id;
    Player* p = Game_get_player(player_id);

    // 1. Initialize Value List
    // Uses delegates to fetch values and names from the Game model
    GUIList_init(&list, p, delegate_get_value_count, delegate_get_player_value,
                 delegate_format_player_value, NULL);
    GUI_SET_POS(&list, 0, 14);
    GUI_SET_SIZE(&list, 118, 44);

    // 2. Initialize Header Label
    GUILabel_init(&player_lbl, p->name);
    GUI_SET_SIZE(&player_lbl, 128, 12);
    GUI_SET_FONT_SIZE(&player_lbl, 7);

    // 3. Register and display page
    Page new_page = {.handle_input = PlayerPage_handle_input, .exit = NULL};
    PageManager_switch_page(&new_page);
    PlayerPage_draw();
}