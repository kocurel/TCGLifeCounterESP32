#include "ChangeHistoryPage.h"

#include "AudioManager.h"
#include "GUIFramework.h"
#include "MenuPage.h"
#include "app/GameDelegates.h"
#include "app/PageManager.h"
#include "model/Game.h"

/* --- Private State --- */
static GUIList history_list = {0};
static int s_current_history_index = 0;
static void (*return_func)(void) = NULL;

/* --- Drawing --- */
static void ChangeHistoryPage_draw() {
    GUIRenderer_clear_buffer();

    // Render the list using delegate-based item drawing
    GUI_DRAW(&history_list);

    GUIRenderer_send_buffer();
}

/* --- Input Handling --- */
static void ChangeHistory_handle_input(ButtonCode button) {
    switch (button) {
        case BUTTON_CODE_UP:
            GUIList_up(&history_list);
            break;

        case BUTTON_CODE_DOWN:
            GUIList_down(&history_list);
            break;

        case BUTTON_CODE_LEFT:
            // Fast scroll up
            for (int i = 0; i < 5; i++) GUIList_up(&history_list);
            break;

        case BUTTON_CODE_RIGHT:
            // Fast scroll down
            for (int i = 0; i < 5; i++) GUIList_down(&history_list);
            break;

        case BUTTON_CODE_CANCEL:
            // Return to previous page or default menu
            if (return_func) {
                return_func();
            } else {
                MenuPage_enter();
            }
            return;

        case BUTTON_CODE_ACCEPT: {
            int target_undo_index = GUIList_get_current_index(&history_list);

            // Revert game state to selected historical point
            Game_seek_history(target_undo_index);
            s_current_history_index = target_undo_index;

            AudioManager_play_sound(SOUND_UI_SELECT);
            ChangeHistoryPage_draw();
            return;
        }
        default:
            break;
    }

    // Sync local tracking with list selection
    s_current_history_index = GUIList_get_current_index(&history_list);
    ChangeHistoryPage_draw();
}

/* --- Page Lifecycle --- */
void ChangeHistoryPage_enter(void (*return_dest)(void)) {
    // 1. Initialize State
    return_func = return_dest;
    s_current_history_index = Game_get_current_undo_index();

    // 2. Initialize UI List
    // Uses custom item drawing delegate from GameDelegates
    GUIList_init(&history_list, NULL, delegate_get_change_history_count,
                 delegate_get_value_change, NULL, HistoryPage_draw_item);

    GUI_SET_SIZE(&history_list, 126, 62);
    GUI_SET_POS(&history_list, 2, 2);

    // 3. Restore Selection and Bounds Check
    history_list.selected_index = s_current_history_index;
    int max_count = Game_get_change_history_count();
    if (history_list.selected_index >= max_count) {
        history_list.selected_index = (max_count > 0) ? max_count - 1 : 0;
    }

    // 4. Page Management
    Page new_page = {0};
    new_page.handle_input = ChangeHistory_handle_input;

    PageManager_switch_page(&new_page);
    ChangeHistoryPage_draw();
}