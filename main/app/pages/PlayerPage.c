#include "PlayerPage.h"

#include <math.h>

#include "GUIFramework.h"
#include "MainPage.h"
#include "PlayerSettingsPage.h"
#include "ValueEditorPage.h"
#include "app/GameDelegates.h"
#include "app/PageManager.h"
#include "model/Game.h"

static GUIList list = {0};
static GUILabel player_lbl = {0};
static int page_player_id = 0;

static void PlayerPage_draw() {
    GUIRenderer_clear_buffer();

    GUI_DRAW(&player_lbl);
    GUIRenderer_draw_horizontal_line(13);
    GUI_DRAW(&list);

    /* Render pagination indicators based on scroll bounds */
    const int ROW_HEIGHT = 11;
    const int visible_rows = list.base.height / ROW_HEIGHT;
    const int current_idx = GUIList_get_current_index(&list);
    const int total = NUMBER_OF_VALUES;

    if (current_idx >= visible_rows) {
        GUIRenderer_draw_pixel(64, list.base.y - 1);
    }
    if (current_idx < total - visible_rows) {
        GUIRenderer_draw_pixel(64, list.base.y + list.base.height + 1);
    }

    GUIRenderer_send_buffer();
}

/**
 * Updates scroll animation and triggers redraw if cursor position changes
 */
static void PlayerPage_on_tick(uint32_t delta_ms) {
    const float old_anim_y = list.anim_y;
    GUIList_tick(&list, delta_ms);

    /* Detect active animation flow */
    if (fabsf(list.anim_y - old_anim_y) > 0.05f) {
        list.needs_redraw = true;
    }

    /* Redraw only if requested by animation or input state */
    if (list.needs_redraw) {
        PlayerPage_draw();
        list.needs_redraw = false;
    }
}

static void PlayerPage_handle_input(ButtonCode button);

/**
 * Commits edited value to the game model and refreshes the page
 */
static void PlayerPage_callback(int32_t value) {
    Game_set_value(value, page_player_id, GUIList_get_current_index(&list));
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
            const int value_index = GUIList_get_current_index(&list);
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

void PlayerPage_enter(int player_id) {
    page_player_id = player_id;

    /* Initialize list with player ID as data source context */
    GUIList_init(&list, (void*)(intptr_t)player_id, delegate_get_value_count,
                 NULL, delegate_format_player_value, NULL);

    GUI_SET_POS(&list, 0, 15);
    GUI_SET_SIZE(&list, 118, 44);

    /* Reset animation state */
    list.anim_y = (float)list.base.y;

    GUILabel_init(&player_lbl, Game_get_player_name(player_id));
    GUI_SET_SIZE(&player_lbl, 128, 12);
    GUI_SET_FONT_SIZE(&player_lbl, 7);
    GUILabel_set_alignment(&player_lbl, GUI_ALIGMNENT_CENTER);

    Page new_page = {.handle_input = PlayerPage_handle_input,
                     .on_tick = PlayerPage_on_tick};
    PageManager_switch_page(&new_page);
    PlayerPage_draw();
}