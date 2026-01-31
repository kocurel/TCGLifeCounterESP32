#include "ValueNamesPage.h"

#include <stdio.h>

#include "AudioManager.h"
#include "GUIFramework.h"
#include "KeyboardPage.h"
#include "MenuPage.h"
#include "app/PageManager.h"
#include "model/Game.h"
#include "model/Settings.h"

/* --- Private State --- */
static GUIList list_names;
static GUILabel title_lbl;
static bool is_initialized = false;

/* Tracks which value index is currently being modified during a keyboard
 * session */
static int s_editing_index = -1;

/* --- GUIList Delegates --- */

static int valuenames_get_count(void* data) {
    // Only allow editing names for standard values, excluding internal
    // Commander Damage indices
    return COMMANDER_DAMAGE_START_INDEX;
}

static char* valuenames_to_string(void* data, int index) {
    static char buffer[32];

    const char* name = Game_get_value_name(index);
    if (!name) name = "???";

    // Display format: "1. HP", "2. Poison", etc.
    snprintf(buffer, sizeof(buffer), "%d. %s", index + 1, name);

    return buffer;
}

/* --- Drawing --- */

static void ValueNamesPage_draw() {
    GUIRenderer_clear_buffer();

    // 1. Header
    GUI_DRAW(&title_lbl);
    GUIRenderer_draw_horizontal_line(13);

    // 2. List
    GUI_DRAW(&list_names);

    // 3. Pagination Indicators
    const int ROW_HEIGHT = 11;
    int visible_rows = list_names.base.height / ROW_HEIGHT;
    int current_idx = GUIList_get_current_index(&list_names);
    int total = valuenames_get_count(NULL);

    // UP Indicator
    if (current_idx >= visible_rows) {
        GUIRenderer_draw_pixel(64, list_names.base.y);
    }
    // DOWN Indicator
    if (current_idx < total - visible_rows) {
        GUIRenderer_draw_pixel(64,
                               list_names.base.y + list_names.base.height + 1);
    }

    GUIRenderer_send_buffer();
}

/* --- Callbacks & Input Handling --- */

/**
 * Triggered when user confirms a name change in KeyboardPage.
 * Synchronizes the runtime game model and persistent NVS storage.
 */
static void on_rename_complete(const char* new_name) {
    if (new_name && s_editing_index >= 0) {
        // Update active game session
        Game_set_value_name(s_editing_index, new_name);

        // Update persistent NVS settings
        SettingsModel_save_value_name(s_editing_index, new_name);
    }

    s_editing_index = -1;
    ValueNamesPage_enter();
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
            // Fast scroll up
            for (int i = 0; i < 4; i++) GUIList_up(&list_names);
            break;
        case BUTTON_CODE_RIGHT:
            // Fast scroll down
            for (int i = 0; i < 4; i++) GUIList_down(&list_names);
            break;

        case BUTTON_CODE_ACCEPT: {
            s_editing_index = GUIList_get_current_index(&list_names);

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
    ValueNamesPage_draw();
}

/* --- Page Lifecycle --- */

void ValueNamesPage_enter(void) {
    if (!is_initialized) {
        // Initialize header
        GUILabel_init(&title_lbl, "VALUE NAMES");
        GUI_SET_FONT_SIZE(&title_lbl, 7);
        GUI_SET_SIZE(&title_lbl, 128, 12);
        GUILabel_set_alignment(&title_lbl, GUI_ALIGMNENT_CENTER);

        // Initialize naming list
        GUIList_init(&list_names, NULL, valuenames_get_count, NULL,
                     valuenames_to_string, NULL);

        GUI_SET_POS(&list_names, 0, 15);
        GUI_SET_SIZE(&list_names, 128, 44);

        is_initialized = true;
    }

    Page page = {.handle_input = ValueNamesPage_handle_input, .exit = NULL};

    PageManager_switch_page(&page);
    ValueNamesPage_draw();
}