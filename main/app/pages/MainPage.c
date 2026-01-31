#include "MainPage.h"

#include <stdio.h>
#include <string.h>

#include "ChangeHistoryPage.h"
#include "CommanderPage.h"
#include "GUIFramework.h"
#include "MenuPage.h"
#include "PlayerPage.h"
#include "ValueEditorPage.h"
#include "app/PageManager.h"
#include "model/Game.h"
#include "model/Settings.h"

/* --- Private Types --- */

typedef struct {
    GUIComponent* focused_component;
    GUIVBox root_vbox;
    GUIHBox row_top;
    GUIHBox row_bot;

    GUIVBox box_p1, box_p2, box_p3, box_p4;
    GUILabel lbl_name_p1, lbl_name_p2, lbl_name_p3, lbl_name_p4;
    GUILabel lbl_hp_p1, lbl_hp_p2, lbl_hp_p3, lbl_hp_p4;

    /* Transaction Buffering: Temporary storage before committing to Game model
     */
    int32_t buffered_hp[4];
    bool is_dirty[4];
    uint32_t idle_timer_ms;
} MainPageData;

/* --- Private State --- */

static MainPageData main_page = {0};
static bool is_initialized = false;
static int current_layout_mode = 0;

/* --- Internal Helpers --- */

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

static void MainPage_commit_changes(int player_id) {
    if (player_id < 0 || player_id > 3 || !main_page.is_dirty[player_id])
        return;

    int32_t current_model_val = Game_get_value(player_id, INDEX_HP);
    int32_t new_val = main_page.buffered_hp[player_id];

    if (current_model_val != new_val) {
        Game_set_value(new_val, player_id, INDEX_HP);
    }

    main_page.is_dirty[player_id] = false;
}

static bool MainPage_is_player_dead(int player_id) {
    GameSettings settings = SettingsModel_get();

    if (settings.dead_at_zero && Game_get_value(player_id, INDEX_HP) <= 0)
        return true;

    if (settings.cmd_dmg_rule) {
        for (int source = 0; source < 4; source++) {
            if (source != player_id &&
                Game_get_commander_damage(player_id, source) >= 21)
                return true;
        }
    }
    return false;
}

/* --- Layout Construction --- */

/**
 * Rebuilds the GUI hierarchy based on player count.
 * Handles 180 degree rotation for players in 2-player mode or top row of
 * 4-player mode.
 */
static void MainPage_rebuild_layout(int player_count) {
    // 1. Reset Containers
    GUIVBox_init(&main_page.root_vbox);
    GUIHBox_init(&main_page.row_top);
    GUIHBox_init(&main_page.row_bot);

    GUIVBox_init(&main_page.box_p1);
    GUIVBox_init(&main_page.box_p2);
    GUIVBox_init(&main_page.box_p3);
    GUIVBox_init(&main_page.box_p4);

    GUI_SET_POS(&main_page.root_vbox, 0, 0);
    GUI_SET_SIZE(&main_page.root_vbox, 128, 64);

    GUIVBox* boxes[] = {&main_page.box_p1, &main_page.box_p2, &main_page.box_p3,
                        &main_page.box_p4};
    GUILabel* names[] = {&main_page.lbl_name_p1, &main_page.lbl_name_p2,
                         &main_page.lbl_name_p3, &main_page.lbl_name_p4};
    GUILabel* hps[] = {&main_page.lbl_hp_p1, &main_page.lbl_hp_p2,
                       &main_page.lbl_hp_p3, &main_page.lbl_hp_p4};

    for (int i = 0; i < 4; i++) {
        GUI_SET_SPACING(boxes[i], 0);
        GUI_SET_PADDING(boxes[i], 4);
        GUILabel_set_alignment(names[i], GUI_ALIGMNENT_CENTER);
        GUILabel_set_alignment(hps[i], GUI_ALIGMNENT_CENTER);
    }

    if (player_count == 2) {
        // Player 1: Rotated (Top)
        GUILabel_upside_down_en(&main_page.lbl_name_p1, true);
        GUILabel_upside_down_en(&main_page.lbl_hp_p1, true);
        GUI_ADD_CHILDREN(&main_page.box_p1, &main_page.lbl_hp_p1,
                         &main_page.lbl_name_p1);

        // Player 2: Normal (Bottom)
        GUILabel_upside_down_en(&main_page.lbl_name_p2, false);
        GUILabel_upside_down_en(&main_page.lbl_hp_p2, false);
        GUI_ADD_CHILDREN(&main_page.box_p2, &main_page.lbl_name_p2,
                         &main_page.lbl_hp_p2);

        GUI_ADD_CHILDREN(&main_page.root_vbox, &main_page.box_p1,
                         &main_page.box_p2);
        GUI_SET_SPACING(&main_page.root_vbox, 2);
        GUI_LINK_VERTICAL(&main_page.box_p1, &main_page.box_p2);

    } else {
        // Top row: Rotated
        for (int i = 0; i < 2; i++) {
            GUILabel_upside_down_en(names[i], true);
            GUILabel_upside_down_en(hps[i], true);
            GUI_ADD_CHILDREN(boxes[i], hps[i], names[i]);
        }
        // Bottom row: Normal
        for (int i = 2; i < 4; i++) {
            GUILabel_upside_down_en(names[i], false);
            GUILabel_upside_down_en(hps[i], false);
            GUI_ADD_CHILDREN(boxes[i], names[i], hps[i]);
        }

        GUI_ADD_CHILDREN(&main_page.row_top, &main_page.box_p1,
                         &main_page.box_p2);
        GUI_ADD_CHILDREN(&main_page.row_bot, &main_page.box_p3,
                         &main_page.box_p4);
        GUI_ADD_CHILDREN(&main_page.root_vbox, &main_page.row_top,
                         &main_page.row_bot);

        GUI_SET_SPACING(&main_page.root_vbox, 2);
        GUI_SET_SPACING(&main_page.row_top, 1);
        GUI_SET_SPACING(&main_page.row_bot, 1);

        // Navigation links for 4-player grid
        GUI_LINK_HORIZONTAL(&main_page.box_p1, &main_page.box_p2);
        GUI_LINK_HORIZONTAL(&main_page.box_p3, &main_page.box_p4);
        GUI_LINK_VERTICAL(&main_page.box_p1, &main_page.box_p3);
        GUI_LINK_VERTICAL(&main_page.box_p2, &main_page.box_p4);
    }

    GUI_UPDATE_LAYOUT(&main_page.root_vbox);
    current_layout_mode = player_count;
    main_page.focused_component = (GUIComponent*)&main_page.box_p1;
}

/* --- Graphics & Rendering --- */

static void MainPage_draw_monarch_indicator(int player_id, uint8_t x, uint8_t y,
                                            bool is_rotated) {
    if (!Game_is_monarch(player_id)) return;

    if (is_rotated) {
        GUIRenderer_draw_line(x, y, x + 4, y);
        GUIRenderer_draw_pixel(x, y + 1);
        GUIRenderer_draw_pixel(x + 2, y + 1);
        GUIRenderer_draw_pixel(x + 4, y + 1);
    } else {
        GUIRenderer_draw_line(x, y, x + 4, y);
        GUIRenderer_draw_pixel(x, y - 1);
        GUIRenderer_draw_pixel(x + 2, y - 1);
        GUIRenderer_draw_pixel(x + 4, y - 1);
    }
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
            int32_t val = main_page.is_dirty[i] ? main_page.buffered_hp[i]
                                                : Game_get_value(i, INDEX_HP);
            snprintf(str, sizeof(str), "%ld", (long)val);
            GUI_SET_TEXT(labels_hp[i], str);
        }
        GUI_SET_TEXT(labels_name[i], Game_get_player_name(i));
    }

    GUIRenderer_clear_buffer();
    GUI_DRAW(&main_page.root_vbox);

    for (int i = 0; i < count; i++) {
        if (Game_is_monarch(i)) {
            uint8_t x, y, w, h;
            GUIComponent_get_xywh((GUIComponent*)boxes[i], &x, &y, &w, &h);
            bool rotated = (i < 2 && count == 4) || (i == 0 && count == 2);
            uint8_t crown_x = x + (w / 2) - 2;

            if (rotated)
                MainPage_draw_monarch_indicator(i, crown_x, y + h - 5, true);
            else
                MainPage_draw_monarch_indicator(i, crown_x, y + 4, false);
        }
    }

    if (main_page.focused_component != NULL) {
        uint8_t x, y, w, h;
        GUIComponent_get_xywh(main_page.focused_component, &x, &y, &w, &h);
        GUIRenderer_draw_frame(x, y, w, h);
    }

    int pid = MainPage_get_focused_player_id();
    if (main_page.is_dirty[pid]) {
        int32_t current_model_val = Game_get_value(pid, INDEX_HP);
        int32_t delta = main_page.buffered_hp[pid] - current_model_val;

        if (delta != 0) {
            char delta_str[16];
            snprintf(delta_str, sizeof(delta_str), "%+ld", (long)delta);

            uint8_t box_w = (strlen(delta_str) * 6) + 6;
            uint8_t box_h = 11;
            uint8_t dx = 64 - (box_w / 2);
            uint8_t dy = 32 - (box_h / 2);

            GUIRenderer_set_color(0);
            GUIRenderer_draw_box(dx, dy, box_w, box_h);
            GUIRenderer_set_color(1);
            GUIRenderer_set_font_size(7);
            GUIRenderer_draw_str(dx + 3, dy + box_h - 3, delta_str);
            GUIRenderer_draw_frame(dx, dy - 1, box_w, box_h);
        }
    }

    GUIRenderer_send_buffer();
}

/* --- Callbacks & Input Handling --- */

static void MainPage_editor_callback(int32_t new_value) {
    int player_id = MainPage_get_focused_player_id();
    Game_set_value(new_value, player_id, INDEX_HP);
    main_page.is_dirty[player_id] = false;
    MainPage_update();
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
            main_page.idle_timer_ms = 0;
            if (settings.quick_dmg_en) {
                int32_t step = (settings.starting_life > 1000)
                                   ? 100
                                   : (settings.starting_life > 100 ? 10 : 1);
                if (!main_page.is_dirty[pid]) {
                    main_page.buffered_hp[pid] = Game_get_value(pid, INDEX_HP);
                    main_page.is_dirty[pid] = true;
                }
                main_page.buffered_hp[pid] -= step;
                MainPage_update();
            } else {
                MainPage_commit_changes(pid);
                PlayerPage_enter(pid);
            }
            break;

        case BUTTON_CODE_SET:
            MainPage_commit_changes(pid);
            if (settings.quick_dmg_en)
                PlayerPage_enter(pid);
            else
                ValueEditorPage_enter(Game_get_player_name(pid), "HP", INDEX_HP,
                                      Game_get_value(pid, INDEX_HP),
                                      MainPage_editor_callback);
            return;

        case BUTTON_CODE_CANCEL:
            MainPage_commit_changes(pid);
            if (settings.cmd_mode_en)
                CommanderPage_enter(pid);
            else
                ChangeHistoryPage_enter(MainPage_enter);
            return;

        case BUTTON_CODE_MENU:
            MainPage_commit_changes(pid);
            MenuPage_enter();
            break;

        default:
            break;
    }

    if (next_focus != NULL) {
        MainPage_commit_changes(pid);
        main_page.focused_component = next_focus;
        MainPage_update();
    }
}

/* --- Lifecycle & Task Logic --- */

void MainPage_on_tick(uint32_t delta_ms) {
    bool changes_pending = false;
    for (int i = 0; i < 4; i++) {
        if (main_page.is_dirty[i]) {
            changes_pending = true;
            break;
        }
    }

    if (changes_pending) {
        main_page.idle_timer_ms += delta_ms;
        if (main_page.idle_timer_ms >= 2000) {
            for (int i = 0; i < 4; i++) MainPage_commit_changes(i);
            main_page.idle_timer_ms = 0;
            MainPage_update();
        }
    }
}

void MainPage_enter() {
    GameSettings settings = SettingsModel_get();

    for (int i = 0; i < 4; i++) {
        main_page.is_dirty[i] = false;
        main_page.buffered_hp[i] = 0;
    }

    if (!is_initialized) {
        GUILabel_init(&main_page.lbl_name_p1, "");
        GUILabel_init(&main_page.lbl_name_p2, "");
        GUILabel_init(&main_page.lbl_name_p3, "");
        GUILabel_init(&main_page.lbl_name_p4, "");
        GUILabel_init(&main_page.lbl_hp_p1, "");
        GUILabel_init(&main_page.lbl_hp_p2, "");
        GUILabel_init(&main_page.lbl_hp_p3, "");
        GUILabel_init(&main_page.lbl_hp_p4, "");

        GUI_SET_FONT_SIZE(&main_page.lbl_name_p1, 6);
        GUI_SET_FONT_SIZE(&main_page.lbl_hp_p1, 10);
        GUI_SET_FONT_SIZE(&main_page.lbl_name_p2, 6);
        GUI_SET_FONT_SIZE(&main_page.lbl_hp_p2, 10);
        GUI_SET_FONT_SIZE(&main_page.lbl_name_p3, 6);
        GUI_SET_FONT_SIZE(&main_page.lbl_hp_p3, 10);
        GUI_SET_FONT_SIZE(&main_page.lbl_name_p4, 6);
        GUI_SET_FONT_SIZE(&main_page.lbl_hp_p4, 10);

        is_initialized = true;
    }

    if (current_layout_mode != settings.player_count) {
        MainPage_rebuild_layout(settings.player_count);
    }

    Page new_page = {.handle_input = MainPage_handle_input,
                     .on_tick = MainPage_on_tick};
    PageManager_switch_page(&new_page);
    MainPage_update();
}