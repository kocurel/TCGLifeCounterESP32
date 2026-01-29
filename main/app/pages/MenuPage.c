#include "MenuPage.h"

#include "ChangeHistoryPage.h"
#include "DicePage.h"
#include "GUIFramework.h"
#include "MainPage.h"
#include "SettingsPage.h"
#include "app/PageManager.h"
typedef struct {
    GUIVBox options;
    GUILabel title_lbl;
    GUILabel settings_lbl;
    GUILabel dice_lbl;
    GUILabel history_lbl;
    GUILabel new_game_lbl;
    GUIComponent* selected_lbl;
} MenuPageData;

static bool is_initialized = false;
static MenuPageData menu_page = {0};

static void MenuPage_draw() {
    GUIRenderer_clear_buffer();
    GUI_DRAW(&menu_page.title_lbl);
    GUI_DRAW(&menu_page.options);
    if (menu_page.selected_lbl != NULL) {
        uint8_t x, y, w, h;
        GUIComponent_get_xywh(menu_page.selected_lbl, &x, &y, &w, &h);
        GUIRenderer_draw_frame(x, y + 1, w, h + 1);
    }
    GUIRenderer_send_buffer();
}
static void MenuPage_handle_input(ButtonCode button) {
    switch (button) {
        case BUTTON_CODE_UP:
            if (menu_page.selected_lbl->nav_up) {
                menu_page.selected_lbl = menu_page.selected_lbl->nav_up;
                MenuPage_draw();
            }
            break;
        case BUTTON_CODE_DOWN:
            if (menu_page.selected_lbl->nav_down) {
                menu_page.selected_lbl = menu_page.selected_lbl->nav_down;
                MenuPage_draw();
            }
            break;
        case BUTTON_CODE_CANCEL:
            MainPage_enter();
            break;
        case BUTTON_CODE_ACCEPT:
            if (menu_page.selected_lbl ==
                (GUIComponent*)&menu_page.history_lbl) {
                ChangeHistoryPage_enter();
            } else if (menu_page.selected_lbl ==
                       (GUIComponent*)&menu_page.dice_lbl) {
                DicePage_enter();
            } else if (menu_page.selected_lbl ==
                       (GUIComponent*)&menu_page.settings_lbl) {
                SettingsPage_enter();
            }
            break;
        default:
            break;
    }
}
void MenuPage_enter() {
    if (!is_initialized) {
        GUILabel_init(&menu_page.title_lbl, "MENU");
        GUILabel_init(&menu_page.dice_lbl, "Dice");
        GUILabel_init(&menu_page.history_lbl, "Change history");
        GUILabel_init(&menu_page.settings_lbl, "Settings");
        GUILabel_init(&menu_page.new_game_lbl, "New game");
        GUIVBox_init(&menu_page.options);
        GUI_SET_FONT_SIZE(&menu_page.title_lbl, 7);
        GUI_SET_FONT_SIZE(&menu_page.dice_lbl, 7);
        GUI_SET_FONT_SIZE(&menu_page.history_lbl, 7);
        GUI_SET_FONT_SIZE(&menu_page.settings_lbl, 7);
        GUI_SET_FONT_SIZE(&menu_page.new_game_lbl, 7);
        GUI_ADD_CHILDREN(&menu_page.options, &menu_page.dice_lbl,
                         &menu_page.history_lbl, &menu_page.settings_lbl,
                         &menu_page.new_game_lbl);
        GUI_SET_SIZE(&menu_page.title_lbl, 128, 8);
        GUI_SET_POS(&menu_page.options, 16, 10);
        GUI_SET_SIZE(&menu_page.options, 96, 52);
        GUI_UPDATE_LAYOUT(&menu_page.options);
        is_initialized = true;
        GUI_LINK_VERTICAL(&menu_page.dice_lbl, &menu_page.history_lbl);
        GUI_LINK_VERTICAL(&menu_page.history_lbl, &menu_page.settings_lbl);
        GUI_LINK_VERTICAL(&menu_page.settings_lbl, &menu_page.new_game_lbl);
        GUI_LINK_VERTICAL(&menu_page.new_game_lbl, &menu_page.dice_lbl);
    }
    menu_page.selected_lbl = (GUIComponent*)&menu_page.dice_lbl;
    Page new_page = {0};
    new_page.handle_input = MenuPage_handle_input;
    PageManager_switch_page(&new_page);
    MenuPage_draw();
}