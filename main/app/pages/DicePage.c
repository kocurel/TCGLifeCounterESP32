#include "DicePage.h"

#include <stdio.h>

#include "AudioManager.h"
#include "GUIFramework.h"
#include "MenuPage.h"
#include "app/PageManager.h"
#include "model/DiceRoller.h"

// UI Components
static GUIList dice_list = {0};
static GUIVBox right_panel = {0};  // Container for right side
static GUILabel qty_label = {0};   // Line 1
static GUILabel res_label = {0};   // Line 2

// State
static int s_num_rolls = 1;
static char s_result_buffer[LABEL_MAX_SIZE] = "-";

// Internal helper for dice names in the list
static int dice_get_count(void* data) {
    return 9;
}  // Updated count for Coin + 8 Dice
static char* dice_item_to_string(void* item, int index) {
    return (char*)DICE_NAMES[index];
}

static void DicePage_draw() {
    GUIRenderer_clear_buffer();

    // 1. Update Quantity Label (Line 1)
    char qty_str[32];
    snprintf(qty_str, sizeof(qty_str), "Count: %d", s_num_rolls);
    GUI_SET_TEXT(&qty_label, qty_str);

    // 2. Update Result Label (Line 2)
    GUI_SET_TEXT(&res_label, s_result_buffer);

    // 3. Draw Components
    // Drawing the VBox automatically draws its children (the labels)
    GUI_DRAW(&dice_list);
    GUI_DRAW(&right_panel);

    // Draw selection frame for the list
    int visual_index = GUIList_get_current_index(&dice_list) % 5;
    GUIRenderer_draw_frame(0, visual_index * 11 + 4, 60, 12);

    GUIRenderer_send_buffer();
}

static void DicePage_handle_input(ButtonCode button) {
    const int sides_map[] = {2, 3, 4, 6, 8, 10, 12, 20, 100};

    switch (button) {
        case BUTTON_CODE_UP:
            GUIList_up(&dice_list);
            break;
        case BUTTON_CODE_DOWN:
            GUIList_down(&dice_list);
            break;
        case BUTTON_CODE_LEFT:
            if (s_num_rolls > 1) s_num_rolls--;
            break;
        case BUTTON_CODE_RIGHT:
            if (s_num_rolls < 9) s_num_rolls++;
            break;
        case BUTTON_CODE_CANCEL:
            MenuPage_enter();
            return;
        case BUTTON_CODE_ACCEPT: {
            AudioManager_play_sound(SOUND_DICE_ROLL);
            int idx = GUIList_get_current_index(&dice_list);
            int sides = sides_map[idx];

            // CASE 1: Single Coin Flip
            if (sides == 2 && s_num_rolls == 1) {
                int res = roll_die(2);
                snprintf(s_result_buffer, LABEL_MAX_SIZE, "%s",
                         (res == 1) ? "Heads" : "Tails");
            }
            // CASE 2: Multiple Coin Flips (Count Heads)
            else if (sides == 2) {
                int heads_count = 0;
                for (int i = 0; i < s_num_rolls; i++) {
                    int res = roll_die(2);
                    if (res == 1) heads_count++;
                }
                snprintf(s_result_buffer, LABEL_MAX_SIZE, "Heads: %d",
                         heads_count);
            }
            // CASE 3: Standard Dice (Sum Total)
            else {
                int total = 0;
                for (int i = 0; i < s_num_rolls; i++) {
                    total += roll_die(sides);
                }
                snprintf(s_result_buffer, LABEL_MAX_SIZE, "Sum: %d", total);
            }
            break;
        }
        default:
            break;
    }
    DicePage_draw();
}

void DicePage_enter() {
    // 1. Initialize List (Left Side)
    GUIList_init(&dice_list, NULL, dice_get_count, NULL, dice_item_to_string,
                 NULL);
    GUI_SET_SIZE(&dice_list, 60, 60);
    GUI_SET_POS(&dice_list, 2, 2);

    // 2. Initialize VBox Container (Right Side)
    GUIVBox_init(&right_panel);
    GUI_SET_POS(&right_panel, 65, 10);
    GUI_SET_SIZE(&right_panel, 60, 50);
    GUI_SET_SPACING(&right_panel, 8);  // Space between Qty and Result

    // 3. Initialize Labels
    GUILabel_init(&qty_label, "Count: 1");
    GUI_SET_FONT_SIZE(&qty_label, 6);
    GUILabel_set_alignment(&qty_label, GUI_ALIGMNENT_LEFT);

    GUILabel_init(&res_label, "Ready");
    GUI_SET_FONT_SIZE(&res_label, 7);  // Make result slightly larger
    GUILabel_set_alignment(&res_label, GUI_ALIGMNENT_LEFT);

    // 4. Build Hierarchy (Add labels to VBox)
    GUI_ADD_CHILD(&right_panel, &qty_label);
    GUI_ADD_CHILD(&right_panel, &res_label);

    // Force layout update to calculate positions of children
    GUI_UPDATE_LAYOUT(&right_panel);

    // 5. Load Page
    Page new_page = {.handle_input = DicePage_handle_input, .exit = NULL};
    PageManager_switch_page(&new_page);
    DicePage_draw();
}