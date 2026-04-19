#include "ValueNamesPage.h"

#include <math.h>
#include <stdio.h>

#include "AudioManager.h"
#include "GUIFramework.h"
#include "KeyboardPage.h"
#include "MenuPage.h"
#include "app/PageManager.h"
#include "model/Game.h"
#include "model/Settings.h"

static GUIList list_names;
static GUILabel title_lbl;
static bool is_initialized = false;
static int s_editing_index = -1;

/**
 * Returns total number of editable game values
 */
static int valuenames_get_count(void* data) {
    return COMMANDER_DAMAGE_START_INDEX;
}

/**
 * Formats value entry as indexed string for list rendering
 */
static char* valuenames_to_string(void* data, int index) {
    static char buffer[32];
    const char* name = Game_get_value_name(index);
    if (!name) name = "???";
    snprintf(buffer, sizeof(buffer), "%d. %s", index + 1, name);
    return buffer;
}

static void ValueNamesPage_draw() {
    GUIRenderer_clear_buffer();

    GUI_DRAW(&title_lbl);
    GUIRenderer_draw_horizontal_line(13);

    GUI_DRAW(&list_names);

    // Render pagination dots based on scroll position
    const int ROW_HEIGHT = 11;
    const int visible_rows = list_names.base.height / ROW_HEIGHT;
    const int current_idx = GUIList_get_current_index(&list_names);
    const int total = valuenames_get_count(NULL);

    if (current_idx >= visible_rows) {
        GUIRenderer_draw_pixel(64, list_names.base.y);
    }
    if (current_idx < total - visible_rows) {
        GUIRenderer_draw_pixel(64,
                               list_names.base.y + list_names.base.height + 1);
    }

    GUIRenderer_send_buffer();
}

/**
 * Persists new name to game model and storage
 */
static void on_rename_complete(const char* new_name) {
    if (new_name && s_editing_index >= 0) {
        Game_set_value_name(s_editing_index, new_name);
        SettingsModel_save_value_name(s_editing_index, new_name);
    }

    s_editing_index = -1;
    ValueNamesPage_enter();
}

/**
 * Updates list animation state and triggers redraw on movement
 */
static void ValueNamesPage_on_tick(uint32_t delta_ms) {
    const float old_y = list_names.anim_y;

    GUIList_tick(&list_names, delta_ms);

    if (fabsf(list_names.anim_y - old_y) > 0.05f) {
        list_names.needs_redraw = true;
    }

    if (list_names.needs_redraw) {
        ValueNamesPage_draw();
        list_names.needs_redraw = false;
    }
}

static void ValueNamesPage_handle_input(ButtonCode button) {
    switch (button) {
        case BUTTON_CODE_UP:
            GUIList_up(&list_names);
            break;
        case BUTTON_CODE_DOWN:
            GUIList_down(&list_names);
            break;

        case BUTTON_CODE_LEFT:
            for (int i = 0; i < 4; i++) GUIList_up(&list_names);
            break;
        case BUTTON_CODE_RIGHT:
            for (int i = 0; i < 4; i++) GUIList_down(&list_names);
            break;

        case BUTTON_CODE_ACCEPT: {
            s_editing_index = GUIList_get_current_index(&list_names);

            // Prevent editing protected commander damage values
            if (s_editing_index >= COMMANDER_DAMAGE_START_INDEX) {
                AudioManager_play_sound(SOUND_UI_ERROR);
                return;
            }

            const char* current_name = Game_get_value_name(s_editing_index);
            KeyboardPage_enter("EDIT VALUE NAME", current_name,
                               on_rename_complete);
            return;
        }

        case BUTTON_CODE_CANCEL:
            MenuPage_enter();
            return;

        default:
            break;
    }
}

void ValueNamesPage_enter(void) {
    if (!is_initialized) {
        GUILabel_init(&title_lbl, "VALUE NAMES");
        GUI_SET_FONT_SIZE(&title_lbl, 7);
        GUI_SET_SIZE(&title_lbl, 128, 12);
        GUILabel_set_alignment(&title_lbl, GUI_ALIGMNENT_CENTER);

        GUIList_init(&list_names, NULL, valuenames_get_count, NULL,
                     valuenames_to_string, NULL);

        GUI_SET_POS(&list_names, 0, 15);
        GUI_SET_SIZE(&list_names, 128, 44);

        is_initialized = true;
    }

    // Force animation snap on entry
    const int visible_rows = list_names.base.height / 11;
    const int relative_row = list_names.selected_index % visible_rows;
    list_names.anim_y = (float)(list_names.base.y + (relative_row * 11));
    list_names.needs_redraw = false;

    Page page = {.handle_input = ValueNamesPage_handle_input,
                 .on_tick = ValueNamesPage_on_tick};

    PageManager_switch_page(&page);
    ValueNamesPage_draw();
}