#include "ChangeHistoryPage.h"

#include "AudioManager.h"
#include "GUIFramework.h"
#include "MenuPage.h"
#include "app/GameDelegates.h"
#include "app/PageManager.h"
#include "model/Game.h"

static GUIList history_list = {0};
static int s_current_history_index = 0;  // Tracks the selected history state
static void (*return_func)(void) = NULL;

static void ChangeHistoryPage_draw() {
    GUIRenderer_clear_buffer();
    GUI_DRAW(&history_list);

    GUIRenderer_send_buffer();
}

static void ChangeHistory_handle_input(ButtonCode button) {
    switch (button) {
        case BUTTON_CODE_UP:
            GUIList_up(&history_list);
            break;

        case BUTTON_CODE_DOWN:
            GUIList_down(&history_list);
            break;

        case BUTTON_CODE_LEFT:
            for (int i = 0; i < 5; i++) GUIList_up(&history_list);
            break;

        case BUTTON_CODE_RIGHT:
            for (int i = 0; i < 5; i++) GUIList_down(&history_list);
            break;

        case BUTTON_CODE_CANCEL:
            if (return_func) {
                return_func();
            } else
                MenuPage_enter();
            return;  // Exit to avoid update after page switch

        case BUTTON_CODE_ACCEPT: {
            int target_undo_index = GUIList_get_current_index(&history_list);
            LOG_DEBUG("ChangeHistory", "Seeking history to index: %d",
                      target_undo_index);

            Game_seek_history(target_undo_index);
            s_current_history_index =
                target_undo_index;  // Sync static state with model

            ChangeHistoryPage_draw();
            AudioManager_play_sound(SOUND_UI_SELECT);
            return;
        }
        default:
            break;
    }

    s_current_history_index =
        GUIList_get_current_index(&history_list);  // Track cursor movement
    ChangeHistoryPage_draw();
}

void ChangeHistoryPage_enter(void (*return_dest)(void)) {
    return_func = return_dest;
    // Sync UI index with current game state (undo steps back)
    s_current_history_index = Game_get_current_undo_index();

    GUIList_init(&history_list, NULL, delegate_get_change_history_count,
                 delegate_get_value_change, NULL, HistoryPage_draw_item);

    GUI_SET_SIZE(&history_list, 126, 62);
    GUI_SET_POS(&history_list, 2, 2);

    // Restore list selection state
    history_list.selected_index = s_current_history_index;

    // Boundary check to prevent index out of bounds
    int max_count = Game_get_change_history_count();
    if (history_list.selected_index >= max_count) {
        history_list.selected_index = (max_count > 0) ? max_count - 1 : 0;
    }

    Page new_page = {0};
    new_page.handle_input = ChangeHistory_handle_input;

    PageManager_switch_page(&new_page);
    ChangeHistoryPage_draw();
}