#include "CommanderPage.h"

#include "GUIFramework.h"
#include "MainPage.h"
#include "app/PageManager.h"
#include "model/Game.h"

typedef struct {
    GUILabel labels[4][4];  //[player_index][damge_source_index]
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

void CommanderPage_clear_selection() {
    if (commander_page.selected_label != NULL) {
        uint8_t x, y, w, h;

        GUIComponent_get_xywh((GUIComponent*)commander_page.selected_label, &x,
                              &y, &w, &h);
        GUIRenderer_set_color(0);
        GUIRenderer_draw_frame(x, y, w, h);
        GUIRenderer_set_color(1);
    }
    GUIRenderer_send_buffer();
}

void CommanderPage_update_selection() {
    if (commander_page.selected_label != NULL) {
        uint8_t x, y, w, h;

        GUIComponent_get_xywh((GUIComponent*)commander_page.selected_label, &x,
                              &y, &w, &h);

        GUIRenderer_draw_frame(x, y, w, h);
    }
    GUIRenderer_send_buffer();
}

void CommanderPage_draw() {
    GUIRenderer_clear_buffer();
    for (int player_id = 0; player_id < 4; player_id++) {
        for (int source_id = 0; source_id < 4; source_id++) {
            GUI_DRAW(&commander_page.labels[player_id][source_id]);
        }
    }
    CommanderPage_update_selection();
    GUIRenderer_send_buffer();
}

void CommanderPage_handle_input(ButtonCode button) {
    switch (button) {
        case BUTTON_CODE_UP:
            if (commander_page.selected_label->base.nav_up != NULL) {
                CommanderPage_clear_selection();
                commander_page.selected_label =
                    (GUILabel*)commander_page.selected_label->base.nav_up;
                CommanderPage_update_selection();
            }
            break;
        case BUTTON_CODE_DOWN:
            if (commander_page.selected_label->base.nav_down != NULL) {
                CommanderPage_clear_selection();
                commander_page.selected_label =
                    (GUILabel*)commander_page.selected_label->base.nav_down;
                CommanderPage_update_selection();
            }
            break;
        case BUTTON_CODE_LEFT:
            if (commander_page.selected_label->base.nav_left != NULL) {
                CommanderPage_clear_selection();
                commander_page.selected_label =
                    (GUILabel*)commander_page.selected_label->base.nav_left;
                CommanderPage_update_selection();
            }
            break;
        case BUTTON_CODE_RIGHT:
            if (commander_page.selected_label->base.nav_right != NULL) {
                CommanderPage_clear_selection();
                commander_page.selected_label =
                    (GUILabel*)commander_page.selected_label->base.nav_right;
                CommanderPage_update_selection();
            }
            break;

        case BUTTON_CODE_ACCEPT:
            ptrdiff_t array_offset =
                commander_page.selected_label - &commander_page.labels[0][0];
            int player_id = array_offset / 4;
            int source_id = array_offset % 4;
            Game_deal_commander_damage(player_id, source_id, 1);
            snprintf(buffer, BUFFER_SIZE, "%ld",
                     Game_get_commander_damage(player_id, source_id));
            GUI_SET_TEXT(&commander_page.labels[player_id][source_id], buffer);
            GUI_DRAW(commander_page.selected_label);
            CommanderPage_clear_selection();
            CommanderPage_update_selection();
            GUIRenderer_send_buffer();
            break;
        default:
            break;
    }
}

void CommanderPage_enter() {
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

    commander_page.selected_label = &commander_page.labels[0][0];
    Page new_page = {0};
    new_page.handle_input = CommanderPage_handle_input;
    PageManager_switch_page(&new_page);
    CommanderPage_draw();
}
