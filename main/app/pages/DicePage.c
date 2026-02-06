#include "DicePage.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "AudioManager.h"
#include "GUIFramework.h"
#include "MenuPage.h"
#include "app/PageManager.h"
#include "model/DiceRoller.h"

/* --- Layout Constants --- */
#define GRID_X 64
#define GRID_Y 12
#define GRID_W 62
#define GRID_H 40

/* --- Private State --- */
static GUIList dice_list = {0};
static GUILabel qty_label = {0};

static GUIVBox grid_main_container;
static GUIHBox grid_row_top;
static GUIHBox grid_row_btm;
static GUILabel result_labels[4];

static int s_dice_count = 1;
static int s_results[4] = {0};
static bool s_has_rolled = false;
static int s_current_sides = 6;
static char s_qty_buffer[16];

static bool s_reroll_mode = false;
static int s_grid_cursor = 0;

static uint32_t s_roll_timer_ms = 0;
static bool s_is_rolling[4] = {false, false, false, false};
#define ROLL_ANIMATION_DURATION 500  // 0.5 sekundy

/* --- Helper Functions --- */

static int dice_get_count(void* data) { return 9; }

static char* dice_item_to_string(void* item, int index) {
    return (char*)DICE_NAMES[index];
}

static void rebuild_grid_topology() {
    grid_main_container.base.count = 0;
    grid_row_top.base.count = 0;
    grid_row_btm.base.count = 0;

    if (s_dice_count == 1) {
        GUI_ADD_CHILD(&grid_main_container, &result_labels[0]);
    } else if (s_dice_count == 2) {
        GUI_ADD_CHILD(&grid_row_top, &result_labels[0]);
        GUI_ADD_CHILD(&grid_row_top, &result_labels[1]);
        GUI_ADD_CHILD(&grid_main_container, &grid_row_top);
    } else {
        GUI_ADD_CHILD(&grid_row_top, &result_labels[0]);
        GUI_ADD_CHILD(&grid_row_top, &result_labels[1]);
        GUI_ADD_CHILD(&grid_row_btm, &result_labels[2]);
        if (s_dice_count == 4) {
            GUI_ADD_CHILD(&grid_row_btm, &result_labels[3]);
        }
        GUI_ADD_CHILD(&grid_main_container, &grid_row_top);
        GUI_ADD_CHILD(&grid_main_container, &grid_row_btm);
    }

    GUI_UPDATE_LAYOUT(&grid_main_container);
}

/* --- Drawing --- */

static void DicePage_draw() {
    GUIRenderer_clear_buffer();

    // 1. Draw Animated Selection List
    GUI_DRAW(&dice_list);

    // 2. Status/Quantity Label
    if (s_reroll_mode) {
        snprintf(s_qty_buffer, sizeof(s_qty_buffer), "[REROLL]");
    } else {
        snprintf(s_qty_buffer, sizeof(s_qty_buffer), "Count: %d", s_dice_count);
    }
    GUI_SET_TEXT(&qty_label, s_qty_buffer);
    GUI_DRAW(&qty_label);

    GUIRenderer_draw_line(62, 0, 62, 64);

    // 3. Results Grid
    if (!s_has_rolled) {
        GUIRenderer_set_font_size(6);
        GUIRenderer_draw_str(GRID_X + 4, GRID_Y + 10, "Press OK");
        GUIRenderer_draw_str(GRID_X + 4, GRID_Y + 20, "to roll");
    } else {
        int sum = 0;
        for (int i = 0; i < s_dice_count; i++) {
            static char buf[4][8];
            if (s_current_sides == 2) {
                snprintf(buf[i], 8, "%s", (s_results[i] == 1) ? "H" : "T");
            } else {
                snprintf(buf[i], 8, "%d", s_results[i]);
                sum += s_results[i];
            }
            GUI_SET_TEXT(&result_labels[i], buf[i]);
        }

        rebuild_grid_topology();

        for (int i = 0; i < s_dice_count; i++) {
            GUI_DRAW(&result_labels[i]);
            GUIRenderer_draw_frame(
                result_labels[i].base.x, result_labels[i].base.y,
                result_labels[i].base.width, result_labels[i].base.height);

            if (s_reroll_mode && s_grid_cursor == i) {
                GUIRenderer_draw_frame(result_labels[i].base.x + 2,
                                       result_labels[i].base.y + 2,
                                       result_labels[i].base.width - 4,
                                       result_labels[i].base.height - 4);

                int x = result_labels[i].base.x;
                int y = result_labels[i].base.y;
                int w = result_labels[i].base.width;
                int h = result_labels[i].base.height;

                GUIRenderer_draw_pixel(x, y);
                GUIRenderer_draw_pixel(x + w - 1, y);
                GUIRenderer_draw_pixel(x, y + h - 1);
                GUIRenderer_draw_pixel(x + w - 1, y + h - 1);
            }
        }

        if (s_current_sides != 2 && s_dice_count > 1) {
            char sum_buf[16];
            snprintf(sum_buf, sizeof(sum_buf), "Total: %d", sum);
            GUIRenderer_set_font_size(6);
            GUIRenderer_draw_str(GRID_X, GRID_Y + GRID_H + 8, sum_buf);
        }
    }

    GUIRenderer_send_buffer();
}

/* --- Lifecycle & Task Logic --- */

static void DicePage_on_tick(uint32_t delta_ms) {
    bool needs_draw = false;

    // 1. Animacja listy (kursor)
    float old_y = dice_list.anim_y;
    GUIList_tick(&dice_list, delta_ms);
    if (fabsf(dice_list.anim_y - old_y) > 0.05f) needs_draw = true;

    // 2. Animacja rzutu kością
    if (s_roll_timer_ms > 0) {
        if (delta_ms >= s_roll_timer_ms) {
            s_roll_timer_ms = 0;
            // Koniec animacji - upewnij się, że s_is_rolling są false
            for (int i = 0; i < 4; i++) s_is_rolling[i] = false;
        } else {
            s_roll_timer_ms -= delta_ms;
            // Podczas trwania rzutu, losuj tymczasowe wyniki dla animowanych
            // kości
            for (int i = 0; i < 4; i++) {
                if (s_is_rolling[i]) {
                    s_results[i] = roll_die(s_current_sides);
                }
            }
        }
        needs_draw = true;
    }

    if (needs_draw) {
        DicePage_draw();
    }
}

/* --- Input Handling --- */

static void DicePage_handle_input(ButtonCode button) {
    const int sides_map[] = {2, 3, 4, 6, 8, 10, 12, 20, 100};

    // Mode: Reroll specific die (Selection mode)
    if (s_reroll_mode) {
        bool changed = false;
        switch (button) {
            case BUTTON_CODE_RIGHT:
                if (s_grid_cursor == 0 && s_dice_count > 1) {
                    s_grid_cursor = 1;
                    changed = true;
                } else if (s_grid_cursor == 2 && s_dice_count > 3) {
                    s_grid_cursor = 3;
                    changed = true;
                }
                break;
            case BUTTON_CODE_LEFT:
                if (s_grid_cursor == 1) {
                    s_grid_cursor = 0;
                    changed = true;
                } else if (s_grid_cursor == 3) {
                    s_grid_cursor = 2;
                    changed = true;
                }
                break;
            case BUTTON_CODE_DOWN:
                if (s_grid_cursor == 0 && s_dice_count > 2) {
                    s_grid_cursor = 2;
                    changed = true;
                } else if (s_grid_cursor == 1 && s_dice_count > 3) {
                    s_grid_cursor = 3;
                    changed = true;
                }
                break;
            case BUTTON_CODE_UP:
                if (s_grid_cursor == 2) {
                    s_grid_cursor = 0;
                    changed = true;
                } else if (s_grid_cursor == 3) {
                    s_grid_cursor = 1;
                    changed = true;
                }
                break;
            case BUTTON_CODE_ACCEPT:
                AudioManager_play_sound(SOUND_DICE_ROLL);
                s_is_rolling[s_grid_cursor] = true;
                s_roll_timer_ms = ROLL_ANIMATION_DURATION;
                // Właściwy wynik rzutu i tak zostanie wylosowany w
                // ostatniej klatce on_tick
                break;
            case BUTTON_CODE_SET:
            case BUTTON_CODE_CANCEL:
                s_reroll_mode = false;
                changed = true;  // Refresh to hide focus rings
                break;
            default:
                break;
        }
        if (changed) DicePage_draw();
        return;
    }

    // Mode: Standard configuration
    switch (button) {
        case BUTTON_CODE_UP:
            GUIList_up(&dice_list);
            // Tu nie dajemy draw, bo on_tick obsłuży animację listy
            break;
        case BUTTON_CODE_DOWN:
            GUIList_down(&dice_list);
            // Tu też nie
            break;

        case BUTTON_CODE_LEFT:
            if (s_dice_count > 1) {
                s_dice_count--;
                s_has_rolled = false;
                DicePage_draw();  // Musimy odświeżyć grid
            }
            break;
        case BUTTON_CODE_RIGHT:
            if (s_dice_count < 4) {
                s_dice_count++;
                s_has_rolled = false;
                DicePage_draw();  // Musimy odświeżyć grid
            }
            break;

        case BUTTON_CODE_CANCEL:
            MenuPage_enter();
            return;

        case BUTTON_CODE_SET:
            if (s_has_rolled) {
                s_reroll_mode = true;
                s_grid_cursor = 0;
                DicePage_draw();  // Pokaż ramkę reroll
            }
            break;

        case BUTTON_CODE_ACCEPT: {
            AudioManager_play_sound(SOUND_DICE_ROLL);
            int idx = GUIList_get_current_index(&dice_list);
            s_current_sides = sides_map[idx];

            // Aktywuj rzut dla wszystkich wybranych kości
            for (int i = 0; i < s_dice_count; i++) {
                s_is_rolling[i] = true;
            }
            s_roll_timer_ms = ROLL_ANIMATION_DURATION;
            s_has_rolled = true;
            break;
        }
        default:
            break;
    }
}

/* --- Entry Point --- */

void DicePage_enter() {
    s_has_rolled = false;
    s_reroll_mode = false;
    s_dice_count = 1;

    GUIList_init(&dice_list, NULL, dice_get_count, NULL, dice_item_to_string,
                 NULL);
    GUI_SET_SIZE(&dice_list, 60, 60);
    GUI_SET_POS(&dice_list, 2, 2);

    // Snap anim_y na wejściu
    dice_list.anim_y = dice_list.base.y;

    GUILabel_init(&qty_label, "Count: 1");
    GUI_SET_FONT_SIZE(&qty_label, 6);
    GUI_SET_POS(&qty_label, GRID_X, 4);
    GUI_SET_SIZE(&qty_label, 60, 8);

    GUIVBox_init(&grid_main_container);
    GUI_SET_POS(&grid_main_container, GRID_X, GRID_Y);
    GUI_SET_SIZE(&grid_main_container, GRID_W, GRID_H);
    GUI_SET_PADDING(&grid_main_container, 0);
    GUI_SET_SPACING(&grid_main_container, 2);

    GUIHBox_init(&grid_row_top);
    GUI_SET_SPACING(&grid_row_top, 2);
    GUIHBox_init(&grid_row_btm);
    GUI_SET_SPACING(&grid_row_btm, 2);

    for (int i = 0; i < 4; i++) {
        GUILabel_init(&result_labels[i], "-");
        GUI_SET_FONT_SIZE(&result_labels[i], 7);
        GUILabel_set_alignment(&result_labels[i], GUI_ALIGMNENT_CENTER);
    }

    Page new_page = {.handle_input = DicePage_handle_input,
                     .on_tick = DicePage_on_tick};
    PageManager_switch_page(&new_page);
    DicePage_draw();
}