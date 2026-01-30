#include "MainPage.h"

#include <stdio.h>

#include "CommanderPage.h"
#include "GUIFramework.h"
#include "MenuPage.h"
#include "PlayerPage.h"
#include "System.h"
#include "ValueEditorPage.h"
#include "app/PageManager.h"
#include "model/Game.h"
#include "model/Settings.h"

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
static int current_layout_mode =
    0;  // Przechowuje informację o aktualnym układzie (2 lub 4)

// --- Internal Helpers ---

static int MainPage_get_focused_player_id() {
    if (main_page.focused_component == (GUIComponent*)&main_page.lbl_p1)
        return 0;
    if (main_page.focused_component == (GUIComponent*)&main_page.lbl_p2)
        return 1;
    if (main_page.focused_component == (GUIComponent*)&main_page.lbl_p3)
        return 2;
    if (main_page.focused_component == (GUIComponent*)&main_page.lbl_p4)
        return 3;
    return 0;
}

static bool MainPage_is_player_dead(int player_id) {
    GameSettings settings = SettingsModel_get();
    if (settings.dead_at_zero && Game_get_value(player_id, 0) <= 0) return true;
    if (settings.cmd_dmg_rule) {
        for (int source = 0; source < 4; source++) {
            if (source != player_id &&
                Game_get_commander_damage(player_id, source) >= 21)
                return true;
        }
    }
    return false;
}

static void MainPage_update();
static void MainPage_handle_input(ButtonCode button);

static void MainPage_editor_callback(int32_t new_value) {
    int player_id = MainPage_get_focused_player_id();
    Game_set_value(new_value, player_id, 0);

    Page new_page = {.handle_input = MainPage_handle_input, .exit = NULL};
    PageManager_switch_page(&new_page);
    MainPage_update();
}

/**
 * Dynamicznie przebudowuje strukturę drzewa GUI w zależności od liczby graczy.
 */
static void MainPage_rebuild_layout(int player_count) {
    // Re-inicjalizacja kontenerów (czyści dzieci)
    GUIVBox_init(&main_page.root_vbox);
    GUIHBox_init(&main_page.row_top);
    GUIHBox_init(&main_page.row_bot);

    GUI_SET_POS(&main_page.root_vbox, 0, 12);
    GUI_SET_SIZE(&main_page.root_vbox, 128, 56);
    GUI_SET_PADDING(&main_page.root_vbox, 4);

    if (player_count == 2) {
        // --- Układ 2 Graczy (1v1 Face-off) ---
        GUILabel_upside_down_en(&main_page.lbl_p1, true);
        GUILabel_upside_down_en(&main_page.lbl_p2, false);

        // Dodajemy etykiety bezpośrednio do VBoxa, jedna pod drugą
        GUI_ADD_CHILDREN(&main_page.root_vbox, &main_page.lbl_p1,
                         &main_page.lbl_p2);
        GUI_SET_SPACING(&main_page.root_vbox, 16);

        // Nawigacja tylko góra/dół
        GUI_LINK_VERTICAL(&main_page.lbl_p1, &main_page.lbl_p2);
        main_page.lbl_p1.base.nav_left = NULL;
        main_page.lbl_p1.base.nav_right = NULL;
        main_page.lbl_p2.base.nav_left = NULL;
        main_page.lbl_p2.base.nav_right = NULL;
    } else {
        // --- Układ 4 Graczy (Standard EDH) ---
        GUILabel_upside_down_en(&main_page.lbl_p1, true);
        GUILabel_upside_down_en(&main_page.lbl_p2, true);
        GUILabel_upside_down_en(&main_page.lbl_p3, false);
        GUILabel_upside_down_en(&main_page.lbl_p4, false);

        GUI_ADD_CHILDREN(&main_page.row_top, &main_page.lbl_p1,
                         &main_page.lbl_p2);
        GUI_ADD_CHILDREN(&main_page.row_bot, &main_page.lbl_p3,
                         &main_page.lbl_p4);
        GUI_ADD_CHILDREN(&main_page.root_vbox, &main_page.row_top,
                         &main_page.row_bot);

        GUI_SET_SPACING(&main_page.root_vbox, 16);
        GUI_SET_SPACING(&main_page.row_top, 16);
        GUI_SET_SPACING(&main_page.row_bot, 16);

        // Pełna nawigacja po siatce
        GUI_LINK_HORIZONTAL(&main_page.lbl_p1, &main_page.lbl_p2);
        GUI_LINK_HORIZONTAL(&main_page.lbl_p3, &main_page.lbl_p4);
        GUI_LINK_VERTICAL(&main_page.lbl_p1, &main_page.lbl_p3);
        GUI_LINK_VERTICAL(&main_page.lbl_p2, &main_page.lbl_p4);
    }

    GUI_UPDATE_LAYOUT(&main_page.root_vbox);
    current_layout_mode = player_count;
    main_page.focused_component = (GUIComponent*)&main_page.lbl_p1;
}

static void MainPage_update() {
    char str[32];
    int count = SettingsModel_get().player_count;
    GUILabel* labels[4] = {&main_page.lbl_p1, &main_page.lbl_p2,
                           &main_page.lbl_p3, &main_page.lbl_p4};

    for (int i = 0; i < count; i++) {
        if (MainPage_is_player_dead(i)) {
            GUI_SET_TEXT(labels[i], "KO");
        } else {
            snprintf(str, sizeof(str), "%ld", (long)Game_get_value(i, 0));
            GUI_SET_TEXT(labels[i], str);
        }
    }

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

        case BUTTON_CODE_ACCEPT:
            PlayerPage_enter(MainPage_get_focused_player_id());
            break;

        case BUTTON_CODE_SET: {
            int pid = MainPage_get_focused_player_id();
            ValueEditorPage_enter(
                Game_get_player_name(pid), Game_get_value_name(0), 0,
                Game_get_value(pid, 0), MainPage_editor_callback);
            return;
        }

        case BUTTON_CODE_CANCEL:
            CommanderPage_enter(MainPage_get_focused_player_id());
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
    GameSettings settings = SettingsModel_get();

    if (!is_initialized) {
        GUILabel_init(&main_page.lbl_p1, "");
        GUILabel_init(&main_page.lbl_p2, "");
        GUILabel_init(&main_page.lbl_p3, "");
        GUILabel_init(&main_page.lbl_p4, "");

        GUILabel_init(&main_page.lbl_battery_level, "");
        GUI_SET_POS(&main_page.lbl_battery_level, 94, 0);
        GUI_SET_SIZE(&main_page.lbl_battery_level, 28, 12);
        GUI_SET_FONT_SIZE(&main_page.lbl_battery_level, 8);
        is_initialized = true;
    }

    // Jeśli liczba graczy w ustawieniach zmieniła się, przebudowujemy layout
    if (current_layout_mode != settings.player_count) {
        MainPage_rebuild_layout(settings.player_count);
    }

    Page new_page = {.handle_input = MainPage_handle_input, .exit = NULL};
    PageManager_switch_page(&new_page);
    MainPage_update();
}