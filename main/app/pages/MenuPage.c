#include "MenuPage.h"

#include <stdio.h>  // for snprintf

#include "AudioManager.h"
#include "ChangeHistoryPage.h"
#include "ConfirmPage.h"
#include "DicePage.h"
#include "GUIFramework.h"
#include "MainPage.h"
#include "SettingsPage.h"
#include "System.h"
#include "ValueNamesPage.h"
#include "app/PageManager.h"
#include "model/Game.h"
// #include "ValueNamesPage.h" // Uncomment when you create this page

// --- 1. Data Definitions ---

// Enum defines the order of items. Easy to reorder later.
typedef enum {
    MENU_OPT_HISTORY = 0,
    MENU_OPT_DICE,
    MENU_OPT_SETTINGS,
    MENU_OPT_NEW_GAME,
    MENU_OPT_VALUE_NAMES,  // <--- New Item
    MENU_OPT_COUNT         // Automatic counter of items
} MenuOption;

// Text labels corresponding to the Enum
static const char* MENU_LABELS[] = {
    "Change history", "Dice", "Settings", "New game", "Value names",
};

typedef struct {
    GUIList menu_list;  // Replaces the VBox and individual Labels
    GUILabel title_lbl;
} MenuPageData;

static bool is_initialized = false;
static MenuPageData menu_page = {0};

// --- 2. Delegates for GUIList ---

static int menu_get_count(void* data) { return MENU_OPT_COUNT; }

static char* menu_item_to_string(void* data, int index) {
    if (index < 0 || index >= MENU_OPT_COUNT) return "???";
    return (char*)MENU_LABELS[index];
}

// --- 3. Drawing ---

static void MenuPage_draw() {
    GUIRenderer_clear_buffer();

    // Draw Title
    GUI_DRAW(&menu_page.title_lbl);

    // Draw List
    GUI_DRAW(&menu_page.menu_list);

    // --- Pagination Indicators (Arrows) ---
    // We replicate the list logic slightly to know if we need arrows
    // Based on LIST_ROW_HEIGHT = 11 defined in GUIList.c
    const int ROW_HEIGHT = 11;
    int list_h = menu_page.menu_list.base.height;
    int visible_rows = list_h / ROW_HEIGHT;
    int current_idx = GUIList_get_current_index(&menu_page.menu_list);

    // Calculate current page (GUIList uses paged scrolling)
    int current_page = current_idx / visible_rows;
    int total_pages = (MENU_OPT_COUNT + visible_rows - 1) / visible_rows;

    // Center X of the list area
    uint8_t arrow_x =
        menu_page.menu_list.base.x + (menu_page.menu_list.base.width / 2);

    // Draw UP Arrow if not on first page
    if (current_page > 0) {
        uint8_t y = menu_page.menu_list.base.y - 1;
        GUIRenderer_draw_pixel(arrow_x, y);
        GUIRenderer_draw_line(arrow_x - 2, y + 2, arrow_x + 2, y + 2);
    }

    // Draw DOWN Arrow if not on last page
    if (current_page < total_pages - 1) {
        uint8_t y = menu_page.menu_list.base.y + list_h + 3;
        GUIRenderer_draw_line(arrow_x - 2, y, arrow_x + 2, y);
        GUIRenderer_draw_pixel(arrow_x, y + 2);
    }

    // --- System Section (Battery) ---
    int bat = System_get_battery_percentage();
    char bat_buf[12];
    snprintf(bat_buf, sizeof(bat_buf), "%d%%", bat);

    GUIRenderer_set_font_size(6);
    GUIRenderer_draw_str(104, 6, bat_buf);

    GUIRenderer_send_buffer();
}

static void on_game_reset_confirmed() {
    Game_reset();
    // Optional: Auto-return to main page after reset?
}

// --- 4. Input Handling ---

static void MenuPage_handle_input(ButtonCode button) {
    switch (button) {
        case BUTTON_CODE_UP:
            GUIList_up(&menu_page.menu_list);
            AudioManager_play_sound(SOUND_UI_MOVE);
            MenuPage_draw();
            break;

        case BUTTON_CODE_DOWN:
            GUIList_down(&menu_page.menu_list);
            AudioManager_play_sound(SOUND_UI_MOVE);
            MenuPage_draw();
            break;

        case BUTTON_CODE_CANCEL:
            MainPage_enter();
            break;

        case BUTTON_CODE_ACCEPT:
            AudioManager_play_sound(SOUND_UI_SELECT);

            // Get the selected index and cast to Enum
            int selected = GUIList_get_current_index(&menu_page.menu_list);

            switch (selected) {
                case MENU_OPT_HISTORY:
                    ChangeHistoryPage_enter(NULL);
                    break;
                case MENU_OPT_DICE:
                    DicePage_enter();
                    break;
                case MENU_OPT_SETTINGS:
                    SettingsPage_enter();
                    break;
                case MENU_OPT_VALUE_NAMES:
                    ValueNamesPage_enter();
                    printf("Entering Value Names Page\n");
                    break;
                case MENU_OPT_NEW_GAME:
                    ConfirmPage_enter("Reset game?", on_game_reset_confirmed);
                    break;
                default:
                    break;
            }
            break;

        default:
            break;
    }
}

// --- 5. Initialization ---

void MenuPage_enter() {
    if (!is_initialized) {
        GUILabel_init(&menu_page.title_lbl, "MENU");
        GUI_SET_FONT_SIZE(&menu_page.title_lbl, 7);
        GUI_SET_SIZE(&menu_page.title_lbl, 128, 8);

        // Initialize the List
        // We don't need a complex data_source pointer, because
        // our data is in the static MENU_LABELS array. We pass NULL.
        GUIList_init(
            &menu_page.menu_list, NULL, menu_get_count, NULL,
            menu_item_to_string,
            NULL);  // No custom draw_item needed, standard string list is fine

        // Set List Dimensions
        // Height 55 allows for exactly 5 items (11px each).
        // If you make it 44 (4 items), the paging logic will kick in for the
        // 5th item.
        GUI_SET_POS(&menu_page.menu_list, 16, 10);
        GUI_SET_SIZE(&menu_page.menu_list, 96, 44);

        is_initialized = true;
    }

    // Reset selection to top when entering?
    // Or keep it where it was? Usually resetting is safer for main menus.
    // menu_page.menu_list.selected_index = 0;

    Page new_page = {0};
    new_page.handle_input = MenuPage_handle_input;
    PageManager_switch_page(&new_page);
    MenuPage_draw();
}