#include "ChangeHistoryPage.h"

#include "GUIFramework.h"
#include "MenuPage.h"
#include "app/GameDelegates.h"
#include "app/PageManager.h"
#include "model/Game.h"

static GUIList history_list = {0};

static void ChangeHistoryPage_update() {
    GUIRenderer_clear_buffer();
    GUI_DRAW(&history_list);
    GUIRenderer_send_buffer();
}

static void ChangeHistory_handle_input(ButtonCode button) {
    switch (button) {
        case BUTTON_CODE_UP:
            GUIList_up(&history_list);
            ChangeHistoryPage_update();
            break;

        case BUTTON_CODE_DOWN:
            GUIList_down(&history_list);
            ChangeHistoryPage_update();
            break;

        case BUTTON_CODE_CANCEL:
            MenuPage_enter();
            break;

        case BUTTON_CODE_ACCEPT:
            GUIList_get_current_index(&history_list);
        default:
            break;
    }
}

void ChangeHistoryPage_enter() {
    GUIList_init(&history_list, NULL, delegate_get_change_history_count,
                 delegate_get_value_change, NULL, HistoryPage_draw_item);
    GUI_SET_SIZE(&history_list, 128, 64);
    Page new_page = {0};
    new_page.handle_input = ChangeHistory_handle_input;
    PageManager_switch_page(&new_page);
    ChangeHistoryPage_update();
}