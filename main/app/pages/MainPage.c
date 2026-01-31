#include "MainPage.h"

#include <stdio.h>

#include "ChangeHistoryPage.h"
#include "CommanderPage.h"
#include "GUIFramework.h"
#include "MenuPage.h"
#include "PlayerPage.h"
#include "ValueEditorPage.h"
#include "app/PageManager.h"
#include "model/Game.h"
#include "model/Settings.h"

typedef struct {
    GUIComponent* focused_component;
    GUIVBox root_vbox;
    GUIHBox row_top;
    GUIHBox row_bot;

    GUIVBox box_p1;
    GUIVBox box_p2;
    GUIVBox box_p3;
    GUIVBox box_p4;

    GUILabel lbl_name_p1;
    GUILabel lbl_name_p2;
    GUILabel lbl_name_p3;
    GUILabel lbl_name_p4;

    GUILabel lbl_hp_p1;
    GUILabel lbl_hp_p2;
    GUILabel lbl_hp_p3;
    GUILabel lbl_hp_p4;

    GUILabel lbl_battery_level;
} MainPageData;

static MainPageData main_page = {0};
static bool is_initialized = false;
static int current_layout_mode = 0;

// --- Internal Helpers ---

static int MainPage_get_focused_player_id() {
    if (main_page.focused_component == (GUIComponent*)&main_page.box_p1)
        return 0;
    if (main_page.focused_component == (GUIComponent*)&main_page.box_p2)
        return 1;
    if (main_page.focused_component == (GUIComponent*)&main_page.box_p3)
        return 2;
    if (main_page.focused_component == (GUIComponent*)&main_page.box_p4)
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

static void MainPage_draw_monarch_indicator(int player_id, uint8_t x, uint8_t y,
                                            bool is_rotated) {
    if (!Game_is_monarch(player_id)) return;

    if (is_rotated) {
        GUIRenderer_draw_pixel(x, y);
        GUIRenderer_draw_pixel(x + 2, y);
        GUIRenderer_draw_pixel(x + 4, y);
        GUIRenderer_draw_line(x, y + 1, x + 4, y + 1);
    } else {
        GUIRenderer_draw_line(x, y, x + 4, y);
        GUIRenderer_draw_pixel(x, y - 1);
        GUIRenderer_draw_pixel(x + 2, y - 1);
        GUIRenderer_draw_pixel(x + 4, y - 1);
    }
}

static void MainPage_editor_callback(int32_t new_value) {
    int player_id = MainPage_get_focused_player_id();
    Game_set_value(new_value, player_id, 0);

    Page new_page = {.handle_input = MainPage_handle_input, .exit = NULL};
    PageManager_switch_page(&new_page);
    MainPage_update();
}

static void MainPage_rebuild_layout(int player_count) {
    // Reset kontenerów
    GUIVBox_init(&main_page.root_vbox);
    GUIHBox_init(&main_page.row_top);
    GUIHBox_init(&main_page.row_bot);

    GUIVBox_init(&main_page.box_p1);
    GUIVBox_init(&main_page.box_p2);
    GUIVBox_init(&main_page.box_p3);
    GUIVBox_init(&main_page.box_p4);

    GUI_SET_POS(&main_page.root_vbox, 0, 0);
    GUI_SET_SIZE(&main_page.root_vbox, 128, 64);
    GUI_SET_PADDING(&main_page.root_vbox, 0);

    GUIVBox* boxes[] = {&main_page.box_p1, &main_page.box_p2, &main_page.box_p3,
                        &main_page.box_p4};
    GUILabel* names[] = {&main_page.lbl_name_p1, &main_page.lbl_name_p2,
                         &main_page.lbl_name_p3, &main_page.lbl_name_p4};
    GUILabel* hps[] = {&main_page.lbl_hp_p1, &main_page.lbl_hp_p2,
                       &main_page.lbl_hp_p3, &main_page.lbl_hp_p4};

    for (int i = 0; i < 4; i++) {
        GUI_SET_SPACING(boxes[i], 0);

        // [FIX] Dodajemy padding, aby ramka (frame) nie dotykała tekstu
        // 2 piksele wystarczą, by "odkleić" tekst od ramki zaznaczenia
        GUI_SET_PADDING(boxes[i], 4);

        GUILabel_set_alignment(names[i], GUI_ALIGMNENT_CENTER);
        GUILabel_set_alignment(hps[i], GUI_ALIGMNENT_CENTER);
    }

    if (player_count == 2) {
        // --- 2 Players ---
        GUILabel_upside_down_en(&main_page.lbl_name_p1, true);
        GUILabel_upside_down_en(&main_page.lbl_hp_p1, true);
        GUI_ADD_CHILD(&main_page.box_p1, &main_page.lbl_hp_p1);
        GUI_ADD_CHILD(&main_page.box_p1, &main_page.lbl_name_p1);

        GUILabel_upside_down_en(&main_page.lbl_name_p2, false);
        GUILabel_upside_down_en(&main_page.lbl_hp_p2, false);
        GUI_ADD_CHILD(&main_page.box_p2, &main_page.lbl_name_p2);
        GUI_ADD_CHILD(&main_page.box_p2, &main_page.lbl_hp_p2);

        GUI_ADD_CHILD(&main_page.root_vbox, &main_page.box_p1);
        GUI_ADD_CHILD(&main_page.root_vbox, &main_page.box_p2);

        // Zmniejszamy odstęp między graczami, bo padding boksów go powiększył
        GUI_SET_SPACING(&main_page.root_vbox, 0);
        GUI_LINK_VERTICAL(&main_page.box_p1, &main_page.box_p2);

    } else {
        // --- 4 Players ---
        for (int i = 0; i < 2; i++) {
            GUILabel_upside_down_en(names[i], true);
            GUILabel_upside_down_en(hps[i], true);
            GUI_ADD_CHILD(boxes[i], hps[i]);
            GUI_ADD_CHILD(boxes[i], names[i]);
        }
        for (int i = 2; i < 4; i++) {
            GUILabel_upside_down_en(names[i], false);
            GUILabel_upside_down_en(hps[i], false);
            GUI_ADD_CHILD(boxes[i], names[i]);
            GUI_ADD_CHILD(boxes[i], hps[i]);
        }

        GUI_ADD_CHILDREN(&main_page.row_top, &main_page.box_p1,
                         &main_page.box_p2);
        GUI_ADD_CHILDREN(&main_page.row_bot, &main_page.box_p3,
                         &main_page.box_p4);
        GUI_ADD_CHILDREN(&main_page.root_vbox, &main_page.row_top,
                         &main_page.row_bot);

        // Przy 4 graczach musimy bardzo ciasno upakować rzędy
        GUI_SET_SPACING(&main_page.root_vbox, 0);
        GUI_SET_SPACING(&main_page.row_top, 1);
        GUI_SET_SPACING(&main_page.row_bot, 1);

        GUI_LINK_HORIZONTAL(&main_page.box_p1, &main_page.box_p2);
        GUI_LINK_HORIZONTAL(&main_page.box_p3, &main_page.box_p4);
        GUI_LINK_VERTICAL(&main_page.box_p1, &main_page.box_p3);
        GUI_LINK_VERTICAL(&main_page.box_p2, &main_page.box_p4);
    }

    GUI_UPDATE_LAYOUT(&main_page.root_vbox);
    current_layout_mode = player_count;
    main_page.focused_component = (GUIComponent*)&main_page.box_p1;
}

static void MainPage_update() {
    char str[32];
    int count = SettingsModel_get().player_count;

    GUILabel* labels_hp[4] = {&main_page.lbl_hp_p1, &main_page.lbl_hp_p2,
                              &main_page.lbl_hp_p3, &main_page.lbl_hp_p4};
    GUILabel* labels_name[4] = {&main_page.lbl_name_p1, &main_page.lbl_name_p2,
                                &main_page.lbl_name_p3, &main_page.lbl_name_p4};
    GUIVBox* boxes[4] = {&main_page.box_p1, &main_page.box_p2,
                         &main_page.box_p3, &main_page.box_p4};

    for (int i = 0; i < count; i++) {
        if (MainPage_is_player_dead(i)) {
            GUI_SET_TEXT(labels_hp[i], "KO");
        } else {
            snprintf(str, sizeof(str), "%ld", (long)Game_get_value(i, 0));
            GUI_SET_TEXT(labels_hp[i], str);
        }
        GUI_SET_TEXT(labels_name[i], Game_get_player_name(i));
    }

    GUIRenderer_clear_buffer();
    GUI_DRAW(&main_page.root_vbox);
    // [USUNIĘTO] GUI_DRAW(&main_page.lbl_battery_level);

    // 4. Rysowanie korony WYCENTROWANEJ NAD imieniem
    for (int i = 0; i < count; i++) {
        if (Game_is_monarch(i)) {
            uint8_t x, y, w, h;
            GUIComponent_get_xywh((GUIComponent*)boxes[i], &x, &y, &w, &h);
            bool rotated = (i < 2 && count == 4) || (i == 0 && count == 2);

            // Wyliczamy środek poziomy boks-u
            uint8_t crown_x =
                x + (w / 2) - 2;  // -2 bo korona ma 5px szerokości

            if (rotated) {
                // Nad imieniem (wizualnie), czyli fizycznie pod nim w VBoxie
                MainPage_draw_monarch_indicator(i, crown_x, y + h - 5, true);
            } else {
                // Nad imieniem (wizualnie), fizycznie nad nim w VBoxie
                MainPage_draw_monarch_indicator(i, crown_x, y + 4, false);
            }
        }
    }

    if (main_page.focused_component != NULL) {
        uint8_t x, y, w, h;
        GUIComponent_get_xywh(main_page.focused_component, &x, &y, &w, &h);
        GUIRenderer_draw_frame(x, y, w, h);
    }
    GUIRenderer_send_buffer();
}

static void MainPage_handle_input(ButtonCode button) {
    GUIComponent* next_focus = NULL;
    GameSettings settings = SettingsModel_get();
    int pid = MainPage_get_focused_player_id();

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
            if (settings.quick_dmg_en) {
                // OBLICZANIE KROKU (Adaptive Step)
                int32_t step = 1;
                if (settings.starting_life > 1000) {
                    step = 100;  // Dla Yu-Gi-Oh!
                } else if (settings.starting_life > 100) {
                    step = 10;  // Dla formatów średnich
                }

                int32_t current_hp = Game_get_value(pid, 0);
                Game_set_value(current_hp - step, pid, 0);
                MainPage_update();
            } else {
                // NORMAL: Wejście w PlayerPage
                PlayerPage_enter(pid);
            }
            break;

        case BUTTON_CODE_SET:
            if (settings.quick_dmg_en) {
                // QUICK DMG: SET wchodzi w PlayerPage
                PlayerPage_enter(pid);
            } else {
                // NORMAL: SET otwiera edytor HP
                ValueEditorPage_enter(
                    Game_get_player_name(pid), Game_get_value_name(0), 0,
                    Game_get_value(pid, 0), MainPage_editor_callback);
            }
            return;

        case BUTTON_CODE_CANCEL:
            if (settings.cmd_mode_en) {
                CommanderPage_enter(pid);
            } else {
                ChangeHistoryPage_enter(MainPage_enter);
                return;
            }
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
        GUILabel_init(&main_page.lbl_name_p1, "");
        GUI_SET_FONT_SIZE(&main_page.lbl_name_p1, 6);
        GUILabel_init(&main_page.lbl_name_p2, "");
        GUI_SET_FONT_SIZE(&main_page.lbl_name_p2, 6);
        GUILabel_init(&main_page.lbl_name_p3, "");
        GUI_SET_FONT_SIZE(&main_page.lbl_name_p3, 6);
        GUILabel_init(&main_page.lbl_name_p4, "");
        GUI_SET_FONT_SIZE(&main_page.lbl_name_p4, 6);

        GUILabel_init(&main_page.lbl_hp_p1, "");
        GUI_SET_FONT_SIZE(&main_page.lbl_hp_p1, 10);
        GUILabel_init(&main_page.lbl_hp_p2, "");
        GUI_SET_FONT_SIZE(&main_page.lbl_hp_p2, 10);
        GUILabel_init(&main_page.lbl_hp_p3, "");
        GUI_SET_FONT_SIZE(&main_page.lbl_hp_p3, 10);
        GUILabel_init(&main_page.lbl_hp_p4, "");
        GUI_SET_FONT_SIZE(&main_page.lbl_hp_p4, 10);

        GUILabel_init(&main_page.lbl_battery_level, "");
        GUI_SET_POS(&main_page.lbl_battery_level, 94, 0);
        GUI_SET_SIZE(&main_page.lbl_battery_level, 28, 10);
        GUI_SET_FONT_SIZE(&main_page.lbl_battery_level, 7);
        is_initialized = true;
    }

    if (current_layout_mode != settings.player_count) {
        MainPage_rebuild_layout(settings.player_count);
    }

    Page new_page = {.handle_input = MainPage_handle_input, .exit = NULL};
    PageManager_switch_page(&new_page);
    MainPage_update();
}