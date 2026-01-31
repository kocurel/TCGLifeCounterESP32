#include "DicePage.h"

#include <stdio.h>
#include <string.h>

#include "AudioManager.h"
#include "GUIFramework.h"
#include "MenuPage.h"
#include "app/PageManager.h"
#include "model/DiceRoller.h"

// --- Layout Constants ---
#define GRID_X 64
#define GRID_Y 12
#define GRID_W 62
#define GRID_H 40

// --- State ---
static GUIList dice_list = {0};
static GUILabel qty_label = {0};

// --- The Grid Structure ---
static GUIVBox grid_main_container;
static GUIHBox grid_row_top;
static GUIHBox grid_row_btm;
static GUILabel result_labels[4];

static int s_dice_count = 1;
static int s_results[4] = {0};
static bool s_has_rolled = false;
static int s_current_sides = 6;
static char s_qty_buffer[16];

// --- [NEW] Reroll State ---
static bool s_reroll_mode = false;  // Are we selecting a specific die?
static int s_grid_cursor = 0;       // Which die is selected (0-3)

// --- Helpers ---
static int dice_get_count(void* data) { return 9; }
static char* dice_item_to_string(void* item, int index) {
    return (char*)DICE_NAMES[index];
}

// --- Layout Engine Logic ---
static void rebuild_grid_topology() {
    // ... [EXISTING CODE - UNCHANGED] ...
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

static void DicePage_draw() {
    GUIRenderer_clear_buffer();

    // 1. Draw Left List
    GUI_DRAW(&dice_list);
    int visual_index = GUIList_get_current_index(&dice_list) % 5;
    // GUIRenderer_draw_frame(0, visual_index * 11 + 4, 60, 12);

    // 2. Draw Qty Label
    if (s_reroll_mode) {
        // Visual feedback that we are in Reroll Mode
        snprintf(s_qty_buffer, sizeof(s_qty_buffer), "[REROLL]");
    } else {
        snprintf(s_qty_buffer, sizeof(s_qty_buffer), "Count: %d", s_dice_count);
    }
    GUI_SET_TEXT(&qty_label, s_qty_buffer);
    GUI_DRAW(&qty_label);

    // Separator Line
    GUIRenderer_draw_line(62, 0, 62, 64);

    // 3. Draw Results
    if (!s_has_rolled) {
        GUIRenderer_set_font_size(6);
        GUIRenderer_draw_str(GRID_X + 4, GRID_Y + 10, "Press OK");
        GUIRenderer_draw_str(GRID_X + 4, GRID_Y + 20, "to roll");
    } else {
        // A. Update Text
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

        // B. Update Layout
        rebuild_grid_topology();

        // C. Draw Labels & Frames
        for (int i = 0; i < s_dice_count; i++) {
            // 1. Rysuj Tekst
            GUI_DRAW(&result_labels[i]);

            // 2. Rysuj Podstawową Ramkę (Outer Frame)
            GUIRenderer_draw_frame(
                result_labels[i].base.x, result_labels[i].base.y,
                result_labels[i].base.width, result_labels[i].base.height);

            // --- [NOWOŚĆ] Reroll Cursor (Focus Ring) ---
            // Zamiast XOR (wypełnienia), rysujemy drugą ramkę w środku
            if (s_reroll_mode && s_grid_cursor == i) {
                // Rysujemy ramkę pomniejszoną o 2 piksele z każdej strony (gap)
                // Wygląda to jak "celownik" na kostce
                GUIRenderer_draw_frame(result_labels[i].base.x + 2,
                                       result_labels[i].base.y + 2,
                                       result_labels[i].base.width - 4,
                                       result_labels[i].base.height - 4);

                // Opcjonalnie: Dodaj kropki w rogach dla efektu "Snajpera"
                // To zużywa tylko 4 piksele więcej, a wygląda "Tech"
                int x = result_labels[i].base.x;
                int y = result_labels[i].base.y;
                int w = result_labels[i].base.width;
                int h = result_labels[i].base.height;

                GUIRenderer_draw_pixel(x, y);          // Top-Left corner fill
                GUIRenderer_draw_pixel(x + w - 1, y);  // Top-Right corner fill
                GUIRenderer_draw_pixel(x, y + h - 1);  // Btm-Left corner fill
                GUIRenderer_draw_pixel(x + w - 1, y + h - 1);  // Btm-Right
            }
        }

        // 4. Draw Sum (Bottom)
        if (s_current_sides != 2 && s_dice_count > 1) {
            char sum_buf[16];
            snprintf(sum_buf, sizeof(sum_buf), "Total: %d", sum);
            GUIRenderer_set_font_size(6);
            GUIRenderer_draw_str(GRID_X, GRID_Y + GRID_H + 8, sum_buf);
        }
    }

    GUIRenderer_send_buffer();
}

static void DicePage_handle_input(ButtonCode button) {
    const int sides_map[] = {2, 3, 4, 6, 8, 10, 12, 20, 100};

    // --- Mode 2: Reroll specific die ---
    if (s_reroll_mode) {
        switch (button) {
            case BUTTON_CODE_RIGHT:
                // Grid Logic: 0->1, 2->3
                if (s_grid_cursor == 0 && s_dice_count > 1)
                    s_grid_cursor = 1;
                else if (s_grid_cursor == 2 && s_dice_count > 3)
                    s_grid_cursor = 3;
                break;

            case BUTTON_CODE_LEFT:
                // Grid Logic: 1->0, 3->2
                if (s_grid_cursor == 1)
                    s_grid_cursor = 0;
                else if (s_grid_cursor == 3)
                    s_grid_cursor = 2;
                break;

            case BUTTON_CODE_DOWN:
                // Grid Logic: 0->2, 1->3
                if (s_grid_cursor == 0 && s_dice_count > 2)
                    s_grid_cursor = 2;
                else if (s_grid_cursor == 1 && s_dice_count > 3)
                    s_grid_cursor = 3;
                break;

            case BUTTON_CODE_UP:
                // Grid Logic: 2->0, 3->1
                if (s_grid_cursor == 2)
                    s_grid_cursor = 0;
                else if (s_grid_cursor == 3)
                    s_grid_cursor = 1;
                break;

            case BUTTON_CODE_ACCEPT:
                // Reroll ONLY the selected die
                AudioManager_play_sound(SOUND_DICE_ROLL);
                s_results[s_grid_cursor] = roll_die(s_current_sides);
                break;

            case BUTTON_CODE_SET:
            case BUTTON_CODE_CANCEL:
                // Exit Reroll Mode
                s_reroll_mode = false;
                break;

            default:
                break;
        }
        DicePage_draw();
        return;  // Early exit to prevent falling through to normal logic
    }

    // --- Mode 1: Configuration (Standard) ---
    switch (button) {
        case BUTTON_CODE_UP:
            GUIList_up(&dice_list);
            break;
        case BUTTON_CODE_DOWN:
            GUIList_down(&dice_list);
            break;

        case BUTTON_CODE_LEFT:
            if (s_dice_count > 1) {
                s_dice_count--;
                s_has_rolled = false;
            }
            break;
        case BUTTON_CODE_RIGHT:
            if (s_dice_count < 4) {
                s_dice_count++;
                s_has_rolled = false;
            }
            break;

        case BUTTON_CODE_CANCEL:
            MenuPage_enter();
            return;

        case BUTTON_CODE_SET:
            // Enter Reroll Mode (only if we have valid results)
            if (s_has_rolled) {
                s_reroll_mode = true;
                s_grid_cursor = 0;  // Reset cursor to first die
            }
            break;

        case BUTTON_CODE_ACCEPT: {
            // Roll ALL dice
            AudioManager_play_sound(SOUND_DICE_ROLL);
            int idx = GUIList_get_current_index(&dice_list);
            s_current_sides = sides_map[idx];
            for (int i = 0; i < s_dice_count; i++)
                s_results[i] = roll_die(s_current_sides);
            s_has_rolled = true;
            break;
        }
        default:
            break;
    }
    DicePage_draw();
}

void DicePage_enter() {
    s_has_rolled = false;
    s_reroll_mode = false;  // Reset mode
    s_dice_count = 1;

    // 1. List Init
    GUIList_init(&dice_list, NULL, dice_get_count, NULL, dice_item_to_string,
                 NULL);
    GUI_SET_SIZE(&dice_list, 60, 60);
    GUI_SET_POS(&dice_list, 2, 2);

    // 2. Qty Label Init
    GUILabel_init(&qty_label, "Count: 1");
    GUI_SET_FONT_SIZE(&qty_label, 6);
    GUI_SET_POS(&qty_label, GRID_X, 4);
    GUI_SET_SIZE(&qty_label, 60, 8);

    // 3. Grid Containers Init
    GUIVBox_init(&grid_main_container);
    GUI_SET_POS(&grid_main_container, GRID_X, GRID_Y);
    GUI_SET_SIZE(&grid_main_container, GRID_W, GRID_H);
    GUI_SET_PADDING(&grid_main_container, 0);
    GUI_SET_SPACING(&grid_main_container, 2);

    GUIHBox_init(&grid_row_top);
    GUI_SET_SPACING(&grid_row_top, 2);

    GUIHBox_init(&grid_row_btm);
    GUI_SET_SPACING(&grid_row_btm, 2);

    // 4. Result Labels Init
    for (int i = 0; i < 4; i++) {
        GUILabel_init(&result_labels[i], "-");
        GUI_SET_FONT_SIZE(&result_labels[i], 7);
        GUILabel_set_alignment(&result_labels[i], GUI_ALIGMNENT_CENTER);
    }

    Page new_page = {.handle_input = DicePage_handle_input, .exit = NULL};
    PageManager_switch_page(&new_page);
    DicePage_draw();
}