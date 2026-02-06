#include "ChangeHistoryPage.h"

#include <math.h>

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
            for (int i = 0; i < 5; i++) GUIList_up(&history_list);
            break;
        case BUTTON_CODE_RIGHT:
            for (int i = 0; i < 5; i++) GUIList_down(&history_list);
            break;

        case BUTTON_CODE_CANCEL:
            if (return_func)
                return_func();
            else
                MenuPage_enter();
            return;

        case BUTTON_CODE_ACCEPT: {
            int target_undo_index = GUIList_get_current_index(&history_list);
            Game_seek_history(target_undo_index);
            AudioManager_play_sound(SOUND_UI_SELECT);
            history_list.needs_redraw =
                true;  // Wymuś odświeżenie dla ramki "Undo pointer"
            return;
        }
        default:
            break;
    }
}

static void ChangeHistoryPage_on_tick(uint32_t delta_ms) {
    const int ROW_HEIGHT = 11;
    int visible_rows = history_list.base.height / ROW_HEIGHT;

    int old_offset =
        (history_list.selected_index / visible_rows) * visible_rows;
    float old_y = history_list.anim_y;

    GUIList_tick(&history_list, delta_ms);

    int new_offset =
        (history_list.selected_index / visible_rows) * visible_rows;

    // Sprawdzenie ruchu LUB zmiany strony LUB wymuszenia przez needs_redraw
    if (fabsf(history_list.anim_y - old_y) > 0.05f ||
        old_offset != new_offset || history_list.needs_redraw) {
        ChangeHistoryPage_draw();
        history_list.needs_redraw = false;
    }
}

/* --- Page Lifecycle --- */
void ChangeHistoryPage_enter(void (*return_dest)(void)) {
    return_func = return_dest;
    int start_index = Game_get_current_undo_index();

    GUIList_init(&history_list, NULL, delegate_get_change_history_count,
                 delegate_get_value_change, NULL, HistoryPage_draw_item);

    GUI_SET_SIZE(&history_list, 126,
                 55);  // Zmniejszamy nieco, by zmieścić paginację
    GUI_SET_POS(&history_list, 2, 5);

    history_list.selected_index = start_index;

    // Snap animacji na startową pozycję kursora
    int visible_rows = history_list.base.height / 11;
    int relative_row = history_list.selected_index % visible_rows;
    history_list.anim_y = history_list.base.y + (relative_row * 11);

    Page new_page = {.handle_input = ChangeHistory_handle_input,
                     .on_tick = ChangeHistoryPage_on_tick};

    PageManager_switch_page(&new_page);
    ChangeHistoryPage_draw();
}