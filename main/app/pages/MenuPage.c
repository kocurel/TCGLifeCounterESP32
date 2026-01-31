#include "MenuPage.h"

#include <stdio.h>

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

/* --- Types and Constants --- */

typedef enum {
    MENU_OPT_HISTORY = 0,
    MENU_OPT_DICE,
    MENU_OPT_SETTINGS,
    MENU_OPT_NEW_GAME,
    MENU_OPT_VALUE_NAMES,
    MENU_OPT_COUNT
} MenuOption;

static const char* MENU_LABELS[] = {
    "Change history", "Dice", "Settings", "New game", "Value names",
};

typedef struct {
    GUIList menu_list;
    GUILabel title_lbl;
} MenuPageData;

/* --- Private State --- */

static bool is_initialized = false;
static MenuPageData menu_page = {0};

/* --- GUIList Delegates --- */

static int menu_get_count(void* data) { return MENU_OPT_COUNT; }

static char* menu_item_to_string(void* data, int index) {
    if (index < 0 || index >= MENU_OPT_COUNT) return "???";
    return (char*)MENU_LABELS[index];
}

/* --- Drawing --- */

static void MenuPage_draw() {
    GUIRenderer_clear_buffer();

    // 1. Render Title
    GUI_DRAW(&menu_page.title_lbl);

    // 2. Render Options List
    GUI_DRAW(&menu_page.menu_list);

    // 3. Render Pagination Indicators (Arrows)
    // Visible rows based on LIST_ROW_HEIGHT = 11
    const int ROW_HEIGHT = 11;
    int list_h = menu_page.menu_list.base.height;
    int visible_rows = list_h / ROW_HEIGHT;
    int current_idx = GUIList_get_current_index(&menu_page.menu_list);

    int current_page = current_idx / visible_rows;
    int total_pages = (MENU_OPT_COUNT + visible_rows - 1) / visible_rows;
    uint8_t arrow_x =
        menu_page.menu_list.base.x + (menu_page.menu_list.base.width / 2);

    // UP Arrow
    if (current_page > 0) {
        uint8_t y = menu_page.menu_list.base.y - 1;
        GUIRenderer_draw_pixel(arrow_x, y);
        GUIRenderer_draw_line(arrow_x - 2, y + 2, arrow_x + 2, y + 2);
    }

    // DOWN Arrow
    if (current_page < total_pages - 1) {
        uint8_t y = menu_page.menu_list.base.y + list_h + 3;
        GUIRenderer_draw_line(arrow_x - 2, y, arrow_x + 2, y);
        GUIRenderer_draw_pixel(arrow_x, y + 2);
    }

    // 4. Render System Info (Battery)
    int bat = System_get_battery_percentage();
    char bat_buf[12];
    snprintf(bat_buf, sizeof(bat_buf), "%d%%", bat);

    GUIRenderer_set_font_size(6);
    GUIRenderer_draw_str(104, 6, bat_buf);

    GUIRenderer_send_buffer();
}

/* --- Callbacks --- */

static void on_game_reset_confirmed() {
    Game_reset();
    MainPage_enter();
}

/* --- Input Handling --- */

static void MenuPage_handle_input(ButtonCode button) {
    switch (button) {
        case BUTTON_CODE_UP:
            GUIList_up(&menu_page.menu_list);
            AudioManager_play_sound(SOUND_UI_MOVE);
            break;

        case BUTTON_CODE_DOWN:
            GUIList_down(&menu_page.menu_list);
            AudioManager_play_sound(SOUND_UI_MOVE);
            break;

        case BUTTON_CODE_CANCEL:
            MainPage_enter();
            return;

        case BUTTON_CODE_ACCEPT:
            AudioManager_play_sound(SOUND_UI_SELECT);
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
                    break;
                case MENU_OPT_NEW_GAME:
                    ConfirmPage_enter("Reset game?", on_game_reset_confirmed,
                                      MenuPage_enter);
                    break;
                default:
                    break;
            }
            return;

        default:
            break;
    }
    MenuPage_draw();
}

/* --- Page Lifecycle --- */

void MenuPage_enter() {
    if (!is_initialized) {
        // Initialize Title Label
        GUILabel_init(&menu_page.title_lbl, "MENU");
        GUI_SET_FONT_SIZE(&menu_page.title_lbl, 7);
        GUI_SET_SIZE(&menu_page.title_lbl, 128, 8);

        // Initialize Main List
        GUIList_init(&menu_page.menu_list, NULL, menu_get_count, NULL,
                     menu_item_to_string, NULL);
        GUI_SET_POS(&menu_page.menu_list, 16, 10);

        // Height 44 allows for 4 items; paging logic handles the 5th
        GUI_SET_SIZE(&menu_page.menu_list, 96, 44);

        is_initialized = true;
    }

    Page new_page = {.handle_input = MenuPage_handle_input, .exit = NULL};

    PageManager_switch_page(&new_page);
    MenuPage_draw();
}