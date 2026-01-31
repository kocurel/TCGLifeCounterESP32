#include "ValueNamesPage.h"

#include <stdio.h>

#include "AudioManager.h"
#include "GUIFramework.h"
#include "KeyboardPage.h"
#include "MenuPage.h"  // To return to menu on cancel
#include "app/PageManager.h"
#include "model/Game.h"
#include "model/Settings.h"  // Required for saving changes to Flash/NVS
// --- State Variables ---
static GUIList list_names;
static GUILabel title_lbl;
static bool is_initialized = false;

// We need to remember which index we are editing because the
// Keyboard callback only gives us the string, not the index.
static int s_editing_index = -1;

// --- GUIList Delegates ---

static int valuenames_get_count(void* data) {
    // Assuming NUMBER_OF_VALUES is defined in Game.h (e.g. 32)
    return COMMANDER_DAMAGE_START_INDEX;
}

static char* valuenames_to_string(void* data, int index) {
    static char buffer[32];

    // Get current name from the model
    const char* name = Game_get_value_name(index);
    if (!name) name = "???";

    // Format: "1. HP", "2. Poison", etc.
    // Adding 1 to index makes it look more user-friendly
    snprintf(buffer, sizeof(buffer), "%d. %s", index + 1, name);

    return buffer;
}

// --- Drawing ---

static void ValueNamesPage_draw() {
    GUIRenderer_clear_buffer();

    GUI_DRAW(&title_lbl);
    GUIRenderer_draw_horizontal_line(13);  // Separator under title

    GUI_DRAW(&list_names);

    // --- Pagination Arrows (Optional Visual Polish) ---
    // Matches the style we added to MenuPage
    const int ROW_HEIGHT = 11;
    int visible_rows = list_names.base.height / ROW_HEIGHT;
    int current_idx = GUIList_get_current_index(&list_names);
    int total = valuenames_get_count(NULL);

    // Draw UP arrow
    if (current_idx >= visible_rows) {
        GUIRenderer_draw_pixel(64, list_names.base.y);
    }
    // Draw DOWN arrow
    if (current_idx < total - visible_rows) {
        GUIRenderer_draw_pixel(64,
                               list_names.base.y + list_names.base.height + 1);
    }

    GUIRenderer_send_buffer();
}

// --- Callbacks ---

// Called when user presses "OK" on the keyboard
static void on_rename_complete(const char* new_name) {
    if (new_name && s_editing_index >= 0) {
        // 1. Update Runtime Model
        Game_set_value_name(s_editing_index, new_name);

        // 2. Persist to Flash (NVS)
        // Assuming you have a function in SettingsModel to save specific value
        // names
        SettingsModel_save_value_name(s_editing_index, new_name);
    }

    // Reset index safety
    s_editing_index = -1;

    // Return to this page
    ValueNamesPage_enter();
}

// --- Input Handling ---

static void ValueNamesPage_handle_input(ButtonCode button) {
    switch (button) {
        case BUTTON_CODE_UP:
            GUIList_up(&list_names);
            ValueNamesPage_draw();
            break;

        case BUTTON_CODE_DOWN:
            GUIList_down(&list_names);
            ValueNamesPage_draw();
            break;

        case BUTTON_CODE_LEFT:
            // Page Up shortcut (jump 4 items)
            for (int i = 0; i < 4; i++) GUIList_up(&list_names);
            ValueNamesPage_draw();
            break;

        case BUTTON_CODE_RIGHT:
            // Page Down shortcut
            for (int i = 0; i < 4; i++) GUIList_down(&list_names);
            ValueNamesPage_draw();
            break;

        case BUTTON_CODE_ACCEPT: {
            s_editing_index = GUIList_get_current_index(&list_names);

            // Safety check: Never let them edit if somehow out of bounds
            if (s_editing_index >= COMMANDER_DAMAGE_START_INDEX) {
                AudioManager_play_sound(SOUND_UI_ERROR);
                return;
            }

            const char* current_name = Game_get_value_name(s_editing_index);

            KeyboardPage_enter("EDIT VALUE NAME", current_name,
                               on_rename_complete);
            break;
        }

        case BUTTON_CODE_CANCEL:
            // Return to main menu
            MenuPage_enter();
            break;

        default:
            break;
    }
}

// --- Initialization ---

void ValueNamesPage_enter(void) {
    if (!is_initialized) {
        // 1. Setup Title
        GUILabel_init(&title_lbl, "VALUE NAMES");
        GUI_SET_FONT_SIZE(&title_lbl, 7);
        GUI_SET_SIZE(&title_lbl, 128, 12);
        GUILabel_set_alignment(&title_lbl, GUI_ALIGMNENT_CENTER);

        // 2. Setup List
        GUIList_init(&list_names,
                     NULL,  // Data source (not needed, using global Game)
                     valuenames_get_count,  // Delegate: Count
                     NULL,                  // Delegate: Get Item (not needed)
                     valuenames_to_string,  // Delegate: To String
                     NULL);  // Delegate: Draw Item (using default)

        // Position below the title line
        GUI_SET_POS(&list_names, 0, 15);
        // Height 44 allows for 4 items (11px each)
        GUI_SET_SIZE(&list_names, 128, 44);

        is_initialized = true;
    }

    Page page = {.handle_input = ValueNamesPage_handle_input, .exit = NULL};
    PageManager_switch_page(&page);
    ValueNamesPage_draw();
}