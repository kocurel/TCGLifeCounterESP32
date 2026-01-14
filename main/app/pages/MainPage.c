#include "MainPage.h"

#include "CommanderPage.h"
#include "GUIFramework.h"
#include "MenuPage.h"
#include "PlayerPage.h"
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
} MainPageData;

static MainPageData main_page = {0};
static bool is_initialized = false;

static void MainPage_update() {
    char str[32];

    snprintf(str, sizeof(str), "%ld", Game_get_value(0, 0));
    GUI_SET_TEXT(&main_page.lbl_p1, str);

    snprintf(str, sizeof(str), "%ld", Game_get_value(1, 0));
    GUI_SET_TEXT(&main_page.lbl_p2, str);

    snprintf(str, sizeof(str), "%ld", Game_get_value(2, 0));
    GUI_SET_TEXT(&main_page.lbl_p3, str);

    snprintf(str, sizeof(str), "%ld", Game_get_value(3, 0));
    GUI_SET_TEXT(&main_page.lbl_p4, str);

    GUIRenderer_clear_buffer();

    GUI_DRAW(&main_page.root_vbox);

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

        case BUTTON_CODE_ACCEPT:
            if (main_page.focused_component ==
                (GUIComponent*)&main_page.lbl_p1) {
                PlayerPage_enter(0);
            } else if (main_page.focused_component ==
                       (GUIComponent*)&main_page.lbl_p2) {
                PlayerPage_enter(1);
            } else if (main_page.focused_component ==
                       (GUIComponent*)&main_page.lbl_p3) {
                PlayerPage_enter(2);
            } else if (main_page.focused_component ==
                       (GUIComponent*)&main_page.lbl_p4) {
                PlayerPage_enter(3);
            }
            break;
        case BUTTON_CODE_CANCEL:
            CommanderPage_enter();
            break;
        case BUTTON_CODE_MENU:
            MenuPage_enter();
            break;
        default:
            break;
    }

    // Only update if a valid neighbor exists in that direction
    if (next_focus != NULL) {
        main_page.focused_component = next_focus;
        MainPage_update();  // Redraw to show new selection
    }
}

void MainPage_enter() {
    if (!is_initialized) {
        GUIVBox_init(&main_page.root_vbox);
        GUIHBox_init(&main_page.row_top);
        GUIHBox_init(&main_page.row_bot);

        GUILabel_init(&main_page.lbl_p1, "");
        GUILabel_init(&main_page.lbl_p2, "");
        GUILabel_init(&main_page.lbl_p3, "");
        GUILabel_init(&main_page.lbl_p4, "");

        // Link Top Row (P1 <-> P2)
        GUI_LINK_HORIZONTAL(&main_page.lbl_p1, &main_page.lbl_p2);

        // Link Bottom Row (P3 <-> P4)
        GUI_LINK_HORIZONTAL(&main_page.lbl_p3, &main_page.lbl_p4);

        // Link Columns (P1 <-> P3) and (P2 <-> P4)
        GUI_LINK_VERTICAL(&main_page.lbl_p1, &main_page.lbl_p3);
        GUI_LINK_VERTICAL(&main_page.lbl_p2, &main_page.lbl_p4);

        // Set initial focus
        main_page.focused_component = (GUIComponent*)&main_page.lbl_p1;

        // 4. Configure Properties
        GUILabel_upside_down_en(&main_page.lbl_p1, true);
        GUILabel_upside_down_en(&main_page.lbl_p2, true);

        GUI_ADD_CHILDREN(&main_page.row_top, &main_page.lbl_p1,
                         &main_page.lbl_p2);

        GUI_ADD_CHILDREN(&main_page.row_bot, &main_page.lbl_p3,
                         &main_page.lbl_p4);

        GUI_ADD_CHILDREN(&main_page.root_vbox, &main_page.row_top,
                         &main_page.row_bot);

        GUI_SET_SIZE(&main_page.root_vbox, 128, 64);
        GUI_SET_PADDING(&main_page.root_vbox, 5);
        GUI_SET_SPACING(&main_page.row_top, 20);
        GUI_SET_SPACING(&main_page.row_bot, 20);

        GUI_UPDATE_LAYOUT(&main_page.root_vbox);

        is_initialized = true;
    }

    Page new_page;
    new_page.handle_input = MainPage_handle_input;
    new_page.exit = NULL;
    PageManager_switch_page(&new_page);

    MainPage_update();
}