#include "MainPage.h"

#include "CommanderPage.h"
#include "GUIFramework.h"
#include "MenuPage.h"
#include "PlayerPage.h"
#include "System.h"
#include "ValueEditorPage.h"  // Added for the editor
#include "app/PageManager.h"
#include "model/Game.h"

typedef struct {
    GUIComponent* focused_component;
    GUIVBox root_vbox;
    GUIHBox row_top;
    GUIHBox row_bot;
    GUILabel lbl_p1;
    GUILabel lbl_p2;
    GUILabel lbl_p3;
    GUILabel lbl_p4;
    GUILabel lbl_battery_level;
} MainPageData;

static MainPageData main_page = {0};
static bool is_initialized = false;

// --- Internal Helpers ---

// Helper to get player index from the currently focused label
static int MainPage_get_focused_player_id() {
    if (main_page.focused_component == (GUIComponent*)&main_page.lbl_p1)
        return 0;
    if (main_page.focused_component == (GUIComponent*)&main_page.lbl_p2)
        return 1;
    if (main_page.focused_component == (GUIComponent*)&main_page.lbl_p3)
        return 2;
    if (main_page.focused_component == (GUIComponent*)&main_page.lbl_p4)
        return 3;
    return 0;  // Fallback
}

static void MainPage_update();
static void MainPage_handle_input(ButtonCode button);

// Callback invoked by ValueEditorPage when the user saves a value
static void MainPage_editor_callback(int32_t new_value) {
    int player_id = MainPage_get_focused_player_id();

    // Save to model (this adds to history automatically)
    Game_set_value(new_value, player_id, 0);

    // Return focus to MainPage
    Page new_page = {.handle_input = MainPage_handle_input, .exit = NULL};
    PageManager_switch_page(&new_page);

    MainPage_update();
}

static void MainPage_update() {
    char str[32];

    snprintf(str, sizeof(str), "%ld", (long)Game_get_value(0, 0));
    GUI_SET_TEXT(&main_page.lbl_p1, str);

    snprintf(str, sizeof(str), "%ld", (long)Game_get_value(1, 0));
    GUI_SET_TEXT(&main_page.lbl_p2, str);

    snprintf(str, sizeof(str), "%ld", (long)Game_get_value(2, 0));
    GUI_SET_TEXT(&main_page.lbl_p3, str);

    snprintf(str, sizeof(str), "%ld", (long)Game_get_value(3, 0));
    GUI_SET_TEXT(&main_page.lbl_p4, str);

    int battery_level = System_get_battery_percentage();
    snprintf(str, sizeof(str), "%d%%", battery_level);
    GUI_SET_TEXT(&main_page.lbl_battery_level, str);

    GUIRenderer_clear_buffer();

    GUI_DRAW(&main_page.root_vbox);
    GUI_DRAW(&main_page.lbl_battery_level);

    if (main_page.focused_component != NULL) {
        uint8_t x, y, w, h;
        GUIComponent_get_xywh(main_page.focused_component, &x, &y, &w, &h);
        GUIRenderer_draw_frame(x, y, w, h);
    }
    GUIRenderer_send_buffer();
}

static void MainPage_handle_input(ButtonCode button) {
    GUIComponent* next_focus = NULL;

    switch (button) {
        case BUTTON_CODE_UP:
            next_focus = main_page.focused_component->nav_up;
            break;
        case BUTTON_CODE_DOWN:
            next_focus = main_page.focused_component->nav_down;
            break;
        case BUTTON_CODE_LEFT:
            next_focus = main_page.focused_component->nav_left;
            break;
        case BUTTON_CODE_RIGHT:
            next_focus = main_page.focused_component->nav_right;
            break;

        case BUTTON_CODE_ACCEPT: {
            int pid = MainPage_get_focused_player_id();
            PlayerPage_enter(pid);
            break;
        }

        case BUTTON_CODE_SET: {
            // Open the advanced editor for the focused player's health
            // (index 0)
            int pid = MainPage_get_focused_player_id();
            ValueEditorPage_enter(
                Game_get_player_name(pid), Game_get_value_name(0),
                Game_get_value(pid, 0), MainPage_editor_callback);
            return;  // Page changed, exit handler
        }

        case BUTTON_CODE_CANCEL:
            int pid = MainPage_get_focused_player_id();
            CommanderPage_enter(pid);
            break;
        case BUTTON_CODE_MENU:
            MenuPage_enter();
            break;
        default:
            break;
    }

    if (next_focus != NULL) {
        main_page.focused_component = next_focus;
        MainPage_update();
    }
}

void MainPage_enter() {
    if (!is_initialized) {
        // ... (All initialization code remains the same as your snippet)
        GUIVBox_init(&main_page.root_vbox);
        GUIHBox_init(&main_page.row_top);
        GUIHBox_init(&main_page.row_bot);

        GUILabel_init(&main_page.lbl_p1, "");
        GUILabel_init(&main_page.lbl_p2, "");
        GUILabel_init(&main_page.lbl_p3, "");
        GUILabel_init(&main_page.lbl_p4, "");
        GUILabel_init(&main_page.lbl_battery_level, "");

        GUI_SET_POS(&main_page.lbl_battery_level, 94, 0);
        GUI_SET_SIZE(&main_page.lbl_battery_level, 28, 12);
        GUI_SET_FONT_SIZE(&main_page.lbl_battery_level, 8);

        GUI_LINK_HORIZONTAL(&main_page.lbl_p1, &main_page.lbl_p2);
        GUI_LINK_HORIZONTAL(&main_page.lbl_p3, &main_page.lbl_p4);
        GUI_LINK_VERTICAL(&main_page.lbl_p1, &main_page.lbl_p3);
        GUI_LINK_VERTICAL(&main_page.lbl_p2, &main_page.lbl_p4);

        main_page.focused_component = (GUIComponent*)&main_page.lbl_p1;

        GUILabel_upside_down_en(&main_page.lbl_p1, true);
        GUILabel_upside_down_en(&main_page.lbl_p2, true);

        GUI_ADD_CHILDREN(&main_page.row_top, &main_page.lbl_p1,
                         &main_page.lbl_p2);
        GUI_ADD_CHILDREN(&main_page.row_bot, &main_page.lbl_p3,
                         &main_page.lbl_p4);
        GUI_ADD_CHILDREN(&main_page.root_vbox, &main_page.row_top,
                         &main_page.row_bot);

        GUI_SET_POS(&main_page.root_vbox, 0, 12);
        GUI_SET_SIZE(&main_page.root_vbox, 128, 56);
        GUI_SET_PADDING(&main_page.root_vbox, 4);
        GUI_SET_SPACING(&main_page.row_top, 16);
        GUI_SET_SPACING(&main_page.row_bot, 16);

        GUI_UPDATE_LAYOUT(&main_page.root_vbox);
        is_initialized = true;
    }

    Page new_page = {.handle_input = MainPage_handle_input, .exit = NULL};
    PageManager_switch_page(&new_page);

    MainPage_update();
}