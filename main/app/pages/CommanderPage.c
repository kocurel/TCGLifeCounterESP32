#include "CommanderPage.h"

#include <stdio.h>

#include "Debug.h"
#include "GUIFramework.h"
#include "MainPage.h"
#include "ValueEditorPage.h"
#include "app/PageManager.h"
#include "model/Game.h"

typedef struct {
    GUILabel labels[4][4];
    GUIVBox root;
    GUIHBox row_top;
    GUIHBox row_bot;
    GUIHBox block_p1;
    GUIHBox block_p2;
    GUIHBox block_p3;
    GUIHBox block_p4;
    GUIVBox block_p10;
    GUIVBox block_p11;
    GUIVBox block_p20;
    GUIVBox block_p21;
    GUIVBox block_p30;
    GUIVBox block_p31;
    GUIVBox block_p40;
    GUIVBox block_p41;
    GUILabel* selected_label;
} CommanderPageData;

static CommanderPageData commander_page = {0};
static bool is_initialized = false;
#define BUFFER_SIZE 24
static char buffer[BUFFER_SIZE];

// --- Internal Helpers ---

static void CommanderPage_get_ids(int* player_id, int* source_id) {
    ptrdiff_t offset =
        commander_page.selected_label - &commander_page.labels[0][0];
    *player_id = offset / 4;
    *source_id = offset % 4;
}

static int32_t CommanderPage_get_value_id(int source_id) {
    // Maps 0-3 to the actual indices in the Player's value array
    return COMMANDER_DAMAGE_START_INDEX + source_id;
}

// Logic to update Cmd Damage AND adjust HP automatically
static void CommanderPage_apply_change(int p_id, int s_id,
                                       int32_t new_cmd_val) {
    int32_t val_id = CommanderPage_get_value_id(s_id);
    int32_t old_cmd_val = Game_get_value(p_id, val_id);

    // Calculate how much life was lost (or gained if undoing)
    int32_t diff = new_cmd_val - old_cmd_val;

    if (diff != 0) {
        // 1. Update the Commander Damage counter
        Game_set_value(new_cmd_val, p_id, val_id);

        // 2. Adjust HP (Index 0) inversely
        // If diff is positive (took damage), HP goes down.
        // If diff is negative (correction), HP goes up.
        int32_t current_hp = Game_get_value(p_id, 0);
        Game_set_value(current_hp - diff, p_id, 0);
    }
}

void CommanderPage_handle_input(ButtonCode button);

void CommanderPage_draw() {
    GUIRenderer_clear_buffer();

    char buf[BUFFER_SIZE];
    for (int p = 0; p < 4; p++) {
        for (int s = 0; s < 4; s++) {
            // Jeśli gracz bije samego siebie -> wyświetlamy ME
            if (p == s) {
                GUI_SET_TEXT(&commander_page.labels[p][s], "ME");
            } else {
                snprintf(buf, BUFFER_SIZE, "%ld",
                         (long)Game_get_commander_damage(p, s));
                GUI_SET_TEXT(&commander_page.labels[p][s], buf);
            }
            GUI_DRAW(&commander_page.labels[p][s]);
        }
    }

    if (commander_page.selected_label != NULL) {
        uint8_t x, y, w, h;
        GUIComponent_get_xywh((GUIComponent*)commander_page.selected_label, &x,
                              &y, &w, &h);
        GUIRenderer_draw_frame(x, y, w, h);
    }
    GUIRenderer_send_buffer();
}

static void CommanderPage_editor_callback(int32_t new_value) {
    int p_id, s_id;
    CommanderPage_get_ids(&p_id, &s_id);

    // Use logic helper to update Cmd + HP
    CommanderPage_apply_change(p_id, s_id, new_value);

    // Return to this page
    Page page = {.handle_input = CommanderPage_handle_input};
    PageManager_switch_page(&page);
    CommanderPage_draw();
}
void CommanderPage_handle_input(ButtonCode button) {
    GUIComponent* next = NULL;

    // Pobieramy ID, żeby sprawdzić czy nie jesteśmy na polu "ME"
    int p_id, s_id;
    CommanderPage_get_ids(&p_id, &s_id);
    bool is_me_field = (p_id == s_id);

    switch (button) {
        case BUTTON_CODE_UP:
            next = commander_page.selected_label->base.nav_up;
            break;
        case BUTTON_CODE_DOWN:
            next = commander_page.selected_label->base.nav_down;
            break;
        case BUTTON_CODE_LEFT:
            next = commander_page.selected_label->base.nav_left;
            break;
        case BUTTON_CODE_RIGHT:
            next = commander_page.selected_label->base.nav_right;
            break;

        case BUTTON_CODE_CANCEL:
            MainPage_enter();
            return;

        case BUTTON_CODE_ACCEPT: {
            if (is_me_field) return;  // BLOKADA: nie można modyfikować ME

            int32_t val_id = CommanderPage_get_value_id(s_id);
            int32_t current = Game_get_value(p_id, val_id);
            CommanderPage_apply_change(p_id, s_id, current + 1);
            CommanderPage_draw();
            break;
        }

        case BUTTON_CODE_SET: {
            if (is_me_field) return;  // BLOKADA: nie można modyfikować ME

            int32_t val_id = CommanderPage_get_value_id(s_id);
            ValueEditorPage_enter(Game_get_player_name(p_id), "Cmd Dmg", val_id,
                                  Game_get_value(p_id, val_id),
                                  CommanderPage_editor_callback);
            break;
        }
        default:
            break;
    }

    if (next) {
        commander_page.selected_label = (GUILabel*)next;
        CommanderPage_draw();
    }
}

void CommanderPage_enter(int initial_player_id) {
    LOG_DEBUG("CommanderPage_enter", "initial_player_id: %d",
              initial_player_id);
    if (!is_initialized) {
        GUIVBox_init(&commander_page.root);
        GUIHBox_init(&commander_page.row_top);
        GUIHBox_init(&commander_page.row_bot);
        GUIHBox_init(&commander_page.block_p1);
        GUIHBox_init(&commander_page.block_p2);
        GUIHBox_init(&commander_page.block_p3);
        GUIHBox_init(&commander_page.block_p4);
        GUIVBox_init(&commander_page.block_p10);
        GUIVBox_init(&commander_page.block_p11);
        GUIVBox_init(&commander_page.block_p20);
        GUIVBox_init(&commander_page.block_p21);
        GUIVBox_init(&commander_page.block_p30);
        GUIVBox_init(&commander_page.block_p31);
        GUIVBox_init(&commander_page.block_p40);
        GUIVBox_init(&commander_page.block_p41);
        for (int player_id = 0; player_id < 4; player_id++) {
            for (int source_id = 0; source_id < 4; source_id++) {
                GUILabel_init(&commander_page.labels[player_id][source_id], "");
                GUI_SET_FONT_SIZE(&commander_page.labels[player_id][source_id],
                                  8);
                snprintf(buffer, BUFFER_SIZE, "%ld",
                         Game_get_commander_damage(player_id, source_id));
                GUI_SET_TEXT(&commander_page.labels[player_id][source_id],
                             buffer);
            }
        }
        // vertical root (4 columns)
        GUI_ADD_CHILDREN(&commander_page.root, &commander_page.row_top,
                         &commander_page.row_bot);
        // 2 top horizontal rows
        GUI_ADD_CHILDREN(&commander_page.row_top, &commander_page.block_p1,
                         &commander_page.block_p2);
        // 2 bottom horizontal rows
        GUI_ADD_CHILDREN(&commander_page.row_bot, &commander_page.block_p3,
                         &commander_page.block_p4);
        // player 1 two vertical columns
        GUI_ADD_CHILDREN(&commander_page.block_p1, &commander_page.block_p10,
                         &commander_page.block_p11);
        GUI_ADD_CHILDREN(&commander_page.block_p2, &commander_page.block_p20,
                         &commander_page.block_p21);
        // player 3 two vertical columns
        GUI_ADD_CHILDREN(&commander_page.block_p3, &commander_page.block_p30,
                         &commander_page.block_p31);
        // player 4 two vertical columns
        GUI_ADD_CHILDREN(&commander_page.block_p4, &commander_page.block_p40,
                         &commander_page.block_p41);
        // player 1 labels s1 s3
        GUI_ADD_CHILDREN(&commander_page.block_p10,
                         &commander_page.labels[0][0],
                         &commander_page.labels[0][2]);
        // player 1 labels s2 s4
        GUI_ADD_CHILDREN(&commander_page.block_p11,
                         &commander_page.labels[0][1],
                         &commander_page.labels[0][3]);
        // player 2 labels s1 s3
        GUI_ADD_CHILDREN(&commander_page.block_p20,
                         &commander_page.labels[1][0],
                         &commander_page.labels[1][2]);
        // player 2 labels s2 s4
        GUI_ADD_CHILDREN(&commander_page.block_p21,
                         &commander_page.labels[1][1],
                         &commander_page.labels[1][3]);
        // player 3 labels s1 s3
        GUI_ADD_CHILDREN(&commander_page.block_p30,
                         &commander_page.labels[2][0],
                         &commander_page.labels[2][2]);
        // player 3 labels s2 s4
        GUI_ADD_CHILDREN(&commander_page.block_p31,
                         &commander_page.labels[2][1],
                         &commander_page.labels[2][3]);
        // player 4 labels s1 s3
        GUI_ADD_CHILDREN(&commander_page.block_p40,
                         &commander_page.labels[3][0],
                         &commander_page.labels[3][2]);
        // player 4 labels s2 s4
        GUI_ADD_CHILDREN(&commander_page.block_p41,
                         &commander_page.labels[3][1],
                         &commander_page.labels[3][3]);

        GUI_SET_SIZE(&commander_page.root, 128, 64);
        GUI_SET_PADDING(&commander_page.root, 2);
        GUI_SET_SPACING(&commander_page.root, 12);
        GUI_SET_SPACING(&commander_page.row_top, 16);
        GUI_SET_SPACING(&commander_page.row_bot, 16);

        GUI_UPDATE_LAYOUT(&commander_page.root);

        // link player 1
        GUI_LINK_VERTICAL(&commander_page.labels[0][0],
                          &commander_page.labels[0][2]);
        GUI_LINK_VERTICAL(&commander_page.labels[0][1],
                          &commander_page.labels[0][3]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[0][0],
                            &commander_page.labels[0][1]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[0][2],
                            &commander_page.labels[0][3]);

        // link player 2
        GUI_LINK_VERTICAL(&commander_page.labels[1][0],
                          &commander_page.labels[1][2]);
        GUI_LINK_VERTICAL(&commander_page.labels[1][1],
                          &commander_page.labels[1][3]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[1][0],
                            &commander_page.labels[1][1]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[1][2],
                            &commander_page.labels[1][3]);

        // link player 3
        GUI_LINK_VERTICAL(&commander_page.labels[2][0],
                          &commander_page.labels[2][2]);
        GUI_LINK_VERTICAL(&commander_page.labels[2][1],
                          &commander_page.labels[2][3]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[2][0],
                            &commander_page.labels[2][1]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[2][2],
                            &commander_page.labels[2][3]);

        // link player 4
        GUI_LINK_VERTICAL(&commander_page.labels[3][0],
                          &commander_page.labels[3][2]);
        GUI_LINK_VERTICAL(&commander_page.labels[3][1],
                          &commander_page.labels[3][3]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[3][0],
                            &commander_page.labels[3][1]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[3][2],
                            &commander_page.labels[3][3]);

        // link player 1 with player 3
        GUI_LINK_VERTICAL(&commander_page.labels[0][2],
                          &commander_page.labels[2][0]);
        GUI_LINK_VERTICAL(&commander_page.labels[0][3],
                          &commander_page.labels[2][1]);
        // link player 1 with player 2
        GUI_LINK_HORIZONTAL(&commander_page.labels[0][1],
                            &commander_page.labels[1][0]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[0][3],
                            &commander_page.labels[1][2]);
        // link player 2 with player 4
        GUI_LINK_VERTICAL(&commander_page.labels[1][2],
                          &commander_page.labels[3][0]);
        GUI_LINK_VERTICAL(&commander_page.labels[1][3],
                          &commander_page.labels[3][1]);
        // link player 3 with player 4
        GUI_LINK_HORIZONTAL(&commander_page.labels[2][1],
                            &commander_page.labels[3][0]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[2][3],
                            &commander_page.labels[3][2]);

        is_initialized = true;
    }
    // SANITY CHECK: Ensure ID is 0-3
    if (initial_player_id < 0 || initial_player_id > 3) {
        initial_player_id = 0;
    }

    commander_page.selected_label =
        &commander_page.labels[initial_player_id][0];

    Page new_page = {0};
    new_page.handle_input = CommanderPage_handle_input;
    PageManager_switch_page(&new_page);
    CommanderPage_draw();
}