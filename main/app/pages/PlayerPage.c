#include "PlayerPage.h"

#include <math.h>

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

    GUI_DRAW(&player_lbl);
    GUIRenderer_draw_horizontal_line(13);
    GUI_DRAW(&list);

    // Pagination Indicators
    const int ROW_HEIGHT = 11;
    int visible_rows = list.base.height / ROW_HEIGHT;
    int current_idx = GUIList_get_current_index(&list);
    int total = NUMBER_OF_VALUES;

    if (current_idx >= visible_rows) {
        GUIRenderer_draw_pixel(64, list.base.y - 1);
    }
    if (current_idx < total - visible_rows) {
        GUIRenderer_draw_pixel(64, list.base.y + list.base.height + 1);
    }

    GUIRenderer_send_buffer();
}

static void PlayerPage_on_tick(uint32_t delta_ms) {
    // 1. Wykonaj krok animacji
    float old_anim_y = list.anim_y;
    GUIList_tick(&list, delta_ms);

    // 2. Sprawdź czy kursor się ruszył (animacja płynie)
    if (fabsf(list.anim_y - old_anim_y) > 0.05f) {
        list.needs_redraw = true;
    }

    // 3. Jeśli lista zgłasza potrzebę odświeżenia (przez animację LUB input)
    if (list.needs_redraw) {
        PlayerPage_draw();
        list.needs_redraw = false;  // Reset flagi po rysowaniu
    }
}

static void PlayerPage_handle_input(ButtonCode button);
/* --- Callbacks & Handlers --- */

/**
 * Returns from the Value Editor and commits the new value to the model
 */
static void PlayerPage_callback(int32_t value) {
    Game_set_value(value, page_player_id, GUIList_get_current_index(&list));

    // Restore page state
    PlayerPage_enter(page_player_id);
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
            for (int i = 0; i < 4; i++) GUIList_up(&list);
            break;
        case BUTTON_CODE_RIGHT:
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
            PlayerSettingsPage_enter(page_player_id);
            return;
        default:
            break;
    }
}

/* --- Page Lifecycle --- */

void PlayerPage_enter(int player_id) {
    page_player_id = player_id;

    // Przekazujemy ID gracza jako data_source (rzutowane na void*)
    GUIList_init(&list, (void*)(intptr_t)player_id, delegate_get_value_count,
                 NULL, delegate_format_player_value, NULL);

    GUI_SET_POS(&list, 0, 15);
    GUI_SET_SIZE(&list, 118, 44);

    // Snap animacji
    list.anim_y = list.base.y;

    GUILabel_init(&player_lbl, Game_get_player_name(player_id));
    GUI_SET_SIZE(&player_lbl, 128, 12);
    GUI_SET_FONT_SIZE(&player_lbl, 7);
    GUILabel_set_alignment(&player_lbl, GUI_ALIGMNENT_CENTER);

    Page new_page = {.handle_input = PlayerPage_handle_input,
                     .on_tick = PlayerPage_on_tick};
    PageManager_switch_page(&new_page);
    PlayerPage_draw();
}