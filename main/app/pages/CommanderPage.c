#include "CommanderPage.h"

#include <stdio.h>
#include <string.h>

#include "Debug.h"
#include "GUIFramework.h"
#include "MainPage.h"
#include "ValueEditorPage.h"
#include "app/PageManager.h"
#include "model/Game.h"

/* --- Constants --- */
#define BUFFER_SIZE 24

/* --- Page State Structure --- */
typedef struct {
    GUILabel labels[4][4];  // [Player Receiving][Source Player]
    GUIVBox root;           // Main container

    // Layout hierarchy blocks
    GUIHBox row_top, row_bot;
    GUIHBox block_p1, block_p2, block_p3, block_p4;
    GUIVBox block_p10, block_p11, block_p20, block_p21;
    GUIVBox block_p30, block_p31, block_p40, block_p41;

    GUILabel* selected_label;

    // --- Transaction Buffering (Quick Edit) ---
    int32_t buffered_val[4][4];  // Temporary storage for uncommitted changes
    bool is_dirty[4][4];         // Flags for cells with pending commits
} CommanderPageData;

static CommanderPageData commander_page = {0};
static bool is_initialized = false;
static char buffer[BUFFER_SIZE];

/* --- Internal Helpers --- */

/**
 * Calculates player and source IDs based on label memory offset
 */
static void CommanderPage_get_ids(int* player_id, int* source_id) {
    ptrdiff_t offset =
        commander_page.selected_label - &commander_page.labels[0][0];
    *player_id = offset / 4;
    *source_id = offset % 4;
}

/**
 * Maps player source (0-3) to internal Game model value index
 */
static int32_t CommanderPage_get_value_id(int source_id) {
    return COMMANDER_DAMAGE_START_INDEX + source_id;
}

/**
 * Commits buffered changes to the Game model and applies inverse HP logic
 */
static void CommanderPage_commit_cell(int p_id, int s_id) {
    if (p_id == s_id || !commander_page.is_dirty[p_id][s_id]) return;

    int32_t new_cmd_val = commander_page.buffered_val[p_id][s_id];

    // Apply hardware limits
    if (new_cmd_val > 999) new_cmd_val = 999;
    if (new_cmd_val < 0) new_cmd_val = 0;

    int32_t val_id = CommanderPage_get_value_id(s_id);
    int32_t old_cmd_val = Game_get_value(p_id, val_id);
    int32_t diff = new_cmd_val - old_cmd_val;

    if (diff != 0) {
        // Update damage tracker
        Game_set_value(new_cmd_val, p_id, val_id);

        // Apply HP reduction (Inverse HP logic: damage up = life down)
        int32_t current_hp = Game_get_value(p_id, INDEX_HP);
        Game_set_value(current_hp - diff, p_id, INDEX_HP);
    }

    commander_page.is_dirty[p_id][s_id] = false;
}

/* --- Drawing --- */

static void CommanderPage_draw() {
    GUIRenderer_clear_buffer();

    char buf[BUFFER_SIZE];

    // 1. Draw Grid Cells
    for (int p = 0; p < 4; p++) {
        for (int s = 0; s < 4; s++) {
            if (p == s) {
                GUI_SET_TEXT(&commander_page.labels[p][s], "ME");
            } else {
                // Prioritize buffer if cell is dirty
                int32_t val = commander_page.is_dirty[p][s]
                                  ? commander_page.buffered_val[p][s]
                                  : Game_get_commander_damage(p, s);
                snprintf(buf, BUFFER_SIZE, "%ld", (long)val);
                GUI_SET_TEXT(&commander_page.labels[p][s], buf);
            }
            GUI_DRAW(&commander_page.labels[p][s]);
        }
    }

    // 2. Selection Frame
    if (commander_page.selected_label != NULL) {
        uint8_t x, y, w, h;
        GUIComponent_get_xywh((GUIComponent*)commander_page.selected_label, &x,
                              &y, &w, &h);
        GUIRenderer_draw_frame(x, y - 1, w, h);

        // 3. Floating Delta Indicator (if dirty)
        int p_id, s_id;
        CommanderPage_get_ids(&p_id, &s_id);

        if (commander_page.is_dirty[p_id][s_id]) {
            int32_t current_model_val = Game_get_commander_damage(p_id, s_id);
            int32_t delta =
                commander_page.buffered_val[p_id][s_id] - current_model_val;

            if (delta != 0) {
                char delta_str[16];
                snprintf(delta_str, sizeof(delta_str), "%+ld", (long)delta);

                uint8_t box_w = (strlen(delta_str) * 6) + 6;
                uint8_t box_h = 12;
                uint8_t dx = 64 - (box_w / 2);
                uint8_t dy = 31 - (box_h / 2);

                GUIRenderer_set_color(0);  // Clear background
                GUIRenderer_draw_box(dx, dy, box_w, box_h);
                GUIRenderer_set_color(1);
                GUIRenderer_draw_frame(dx, dy, box_w, box_h);
                GUIRenderer_set_font_size(7);
                GUIRenderer_draw_str(dx + 3, dy + box_h - 3, delta_str);
            }
        }
    }

    GUIRenderer_send_buffer();
}

/* --- Editor Callback --- */

static void CommanderPage_editor_callback(int32_t new_value) {
    int p_id, s_id;
    CommanderPage_get_ids(&p_id, &s_id);

    int32_t val_id = CommanderPage_get_value_id(s_id);
    int32_t old_cmd_val = Game_get_value(p_id, val_id);
    int32_t diff = new_value - old_cmd_val;

    // Apply changes directly to model (SET bypasses buffer)
    Game_set_value(new_value, p_id, val_id);
    if (diff != 0) {
        int32_t current_hp = Game_get_value(p_id, INDEX_HP);
        Game_set_value(current_hp - diff, p_id, INDEX_HP);
    }

    commander_page.is_dirty[p_id][s_id] = false;

    // Page state restoration
    CommanderPage_draw();
}

/* --- Input Handling --- */

static void CommanderPage_handle_input(ButtonCode button) {
    GUIComponent* next = NULL;
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
            CommanderPage_commit_cell(p_id, s_id);
            MainPage_enter();
            return;

        case BUTTON_CODE_ACCEPT:
            if (is_me_field) return;
            if (!commander_page.is_dirty[p_id][s_id]) {
                commander_page.buffered_val[p_id][s_id] =
                    Game_get_commander_damage(p_id, s_id);
                commander_page.is_dirty[p_id][s_id] = true;
            }
            commander_page.buffered_val[p_id][s_id]++;
            CommanderPage_draw();
            break;

        case BUTTON_CODE_SET:
            if (is_me_field) return;
            CommanderPage_commit_cell(p_id, s_id);
            ValueEditorPage_enter(Game_get_player_name(p_id), "Cmd Dmg",
                                  CommanderPage_get_value_id(s_id),
                                  Game_get_commander_damage(p_id, s_id),
                                  CommanderPage_editor_callback);
            break;

        default:
            break;
    }

    if (next) {
        // Auto-commit on context change (navigation)
        CommanderPage_commit_cell(p_id, s_id);
        commander_page.selected_label = (GUILabel*)next;
        CommanderPage_draw();
    }
}

/* --- Lifecycle Management --- */

void CommanderPage_enter(int initial_player_id) {
    // Reset transient buffer state
    memset(commander_page.is_dirty, 0, sizeof(commander_page.is_dirty));

    if (!is_initialized) {
        // 1. Initialize Container Hierarchy
        GUIVBox_init(&commander_page.root);
        GUIHBox_init(&commander_page.row_top);
        GUIHBox_init(&commander_page.row_bot);

        // Player blocks initialization
        GUIHBox_init(&commander_page.block_p1);
        GUIHBox_init(&commander_page.block_p2);
        GUIHBox_init(&commander_page.block_p3);
        GUIHBox_init(&commander_page.block_p4);

        // Sub-column initialization
        GUIVBox_init(&commander_page.block_p10);
        GUIVBox_init(&commander_page.block_p11);
        GUIVBox_init(&commander_page.block_p20);
        GUIVBox_init(&commander_page.block_p21);
        GUIVBox_init(&commander_page.block_p30);
        GUIVBox_init(&commander_page.block_p31);
        GUIVBox_init(&commander_page.block_p40);
        GUIVBox_init(&commander_page.block_p41);

        // 2. Initialize Label Grid
        for (int p = 0; p < 4; p++) {
            for (int s = 0; s < 4; s++) {
                GUILabel_init(&commander_page.labels[p][s], "");
                GUI_SET_FONT_SIZE(&commander_page.labels[p][s], 8);
                GUILabel_set_alignment(&commander_page.labels[p][s],
                                       GUI_ALIGMNENT_CENTER);
            }
        }

        // 3. Construct Hierarchy (Nesting blocks)
        GUI_ADD_CHILDREN(&commander_page.root, &commander_page.row_top,
                         &commander_page.row_bot);
        GUI_ADD_CHILDREN(&commander_page.row_top, &commander_page.block_p1,
                         &commander_page.block_p2);
        GUI_ADD_CHILDREN(&commander_page.row_bot, &commander_page.block_p3,
                         &commander_page.block_p4);

        GUI_ADD_CHILDREN(&commander_page.block_p1, &commander_page.block_p10,
                         &commander_page.block_p11);
        GUI_ADD_CHILDREN(&commander_page.block_p2, &commander_page.block_p20,
                         &commander_page.block_p21);
        GUI_ADD_CHILDREN(&commander_page.block_p3, &commander_page.block_p30,
                         &commander_page.block_p31);
        GUI_ADD_CHILDREN(&commander_page.block_p4, &commander_page.block_p40,
                         &commander_page.block_p41);

        // Add labels to sub-columns (S1/S3 in col 0, S2/S4 in col 1)
        for (int p = 0; p < 4; p++) {
            GUIVBox* col0 = (p == 0)   ? &commander_page.block_p10
                            : (p == 1) ? &commander_page.block_p20
                            : (p == 2) ? &commander_page.block_p30
                                       : &commander_page.block_p40;
            GUIVBox* col1 = (p == 0)   ? &commander_page.block_p11
                            : (p == 1) ? &commander_page.block_p21
                            : (p == 2) ? &commander_page.block_p31
                                       : &commander_page.block_p41;
            GUI_ADD_CHILDREN(col0, &commander_page.labels[p][0],
                             &commander_page.labels[p][2]);
            GUI_ADD_CHILDREN(col1, &commander_page.labels[p][1],
                             &commander_page.labels[p][3]);
        }

        // 4. Global Layout Configuration
        GUI_SET_SIZE(&commander_page.root, 128, 64);
        GUI_SET_PADDING(&commander_page.root, 2);
        GUI_SET_SPACING(&commander_page.root, 12);
        GUI_SET_SPACING(&commander_page.row_top, 16);
        GUI_SET_SPACING(&commander_page.row_bot, 16);
        GUI_UPDATE_LAYOUT(&commander_page.root);

        // 5. Build Bi-directional Navigation Links
        for (int p = 0; p < 4; p++) {
            GUI_LINK_VERTICAL(&commander_page.labels[p][0],
                              &commander_page.labels[p][2]);
            GUI_LINK_VERTICAL(&commander_page.labels[p][1],
                              &commander_page.labels[p][3]);
            GUI_LINK_HORIZONTAL(&commander_page.labels[p][0],
                                &commander_page.labels[p][1]);
            GUI_LINK_HORIZONTAL(&commander_page.labels[p][2],
                                &commander_page.labels[p][3]);
        }

        // Inter-player navigation links
        GUI_LINK_VERTICAL(&commander_page.labels[0][2],
                          &commander_page.labels[2][0]);
        GUI_LINK_VERTICAL(&commander_page.labels[0][3],
                          &commander_page.labels[2][1]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[0][1],
                            &commander_page.labels[1][0]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[0][3],
                            &commander_page.labels[1][2]);
        GUI_LINK_VERTICAL(&commander_page.labels[1][2],
                          &commander_page.labels[3][0]);
        GUI_LINK_VERTICAL(&commander_page.labels[1][3],
                          &commander_page.labels[3][1]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[2][1],
                            &commander_page.labels[3][0]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[2][3],
                            &commander_page.labels[3][2]);

        is_initialized = true;
    }

    commander_page.selected_label =
        &commander_page.labels[(initial_player_id % 4)][0];

    Page new_page = {.handle_input = CommanderPage_handle_input};
    PageManager_switch_page(&new_page);
    CommanderPage_draw();
}