#include "ViewModel.h"

#include <memory.h>
#include <stdio.h>

#include "GUIFramework.h"
#include "Game.h"
#include "Utils.h"

typedef struct {
    int selected_index;
    GUIVBox* root_vbox;
    GUIHBox* row_top;
    GUIHBox* row_bot;
    GUILabel* lbl_p1;
    GUILabel* lbl_p2;
    GUILabel* lbl_p3;
    GUILabel* lbl_p4;
} MainPageData;

typedef struct Page Page;

struct Page {
    void (*handle_input)(ButtonCode);
    void (*exit)();
    void* data;
};

static Page current_page = {
    .handle_input = NULL,
    .exit = NULL,
    .data = NULL,
};

void ViewModel_handle_input(ButtonCode button) {
    if (current_page.handle_input == NULL) {
        return;
    }
    current_page.handle_input(button);
}

static void MainPage_delete() {
    MainPageData* data = (MainPageData*)current_page.data;
    if (data == NULL) return;
    GUI_DELETE(data->root_vbox);
    free(data);
    current_page.data = NULL;
}

static void MainPage_update() {
    MainPageData* data = (MainPageData*)current_page.data;
    char str[15];
    sprintf(str, "%ld", Game_get_player_life(0));
    GUI_SET_TEXT(data->lbl_p1, str);

    sprintf(str, "%ld", Game_get_player_life(1));
    GUI_SET_TEXT(data->lbl_p2, str);

    sprintf(str, "%ld", Game_get_player_life(2));
    GUI_SET_TEXT(data->lbl_p3, str);

    sprintf(str, "%ld", Game_get_player_life(3));
    GUI_SET_TEXT(data->lbl_p4, str);

    GUIRenderer_clear_buffer();
    GUI_DRAW(data->root_vbox);
    int selected_label_id = data->selected_index;
    uint8_t x, y, w, h;
    if (selected_label_id == 1)
        GUIComponent_get_xywh(data->lbl_p1, &x, &y, &w, &h);
    else if (selected_label_id == 2)
        GUIComponent_get_xywh(data->lbl_p2, &x, &y, &w, &h);
    else if (selected_label_id == 3)
        GUIComponent_get_xywh(data->lbl_p3, &x, &y, &w, &h);
    else if (selected_label_id == 4)
        GUIComponent_get_xywh(data->lbl_p4, &x, &y, &w, &h);

    GUIRenderer_draw_frame(x, y, w, h);
    GUIRenderer_send_buffer();
}

static void MainPage_handle_input(ButtonCode button) {
    MainPageData* data = (MainPageData*)current_page.data;
    int selected_label = data->selected_index;
    switch (button) {
        case BUTTON_CODE_ACCEPT:
            /* code */
            break;
        case BUTTON_CODE_CANCEL:
            /* code */
            break;
        case BUTTON_CODE_MENU:
            /* code */
            break;
        case BUTTON_CODE_SET:
            /* code */
            break;
        case BUTTON_CODE_LEFT:
            if (selected_label == 2) {
                data->selected_index = 1;
            }
            if (selected_label == 4) {
                data->selected_index = 3;
            }
            break;
        case BUTTON_CODE_RIGHT:
            if (selected_label == 1) {
                data->selected_index = 2;
            }
            if (selected_label == 3) {
                data->selected_index = 4;
            }
            break;
        case BUTTON_CODE_UP:
            if (selected_label == 3) {
                data->selected_index = 1;
            }
            if (selected_label == 4) {
                data->selected_index = 2;
            }
            break;
        case BUTTON_CODE_DOWN:
            if (selected_label == 1) {
                data->selected_index = 3;
            }
            if (selected_label == 2) {
                data->selected_index = 4;
            }
            break;
        default:
            break;
    }
    MainPage_update();
}

static void MainPage_set() {
    MainPageData* data = (MainPageData*)malloc(sizeof(MainPageData));
    data->selected_index = 1;
    data->root_vbox = GUIVBox_new();
    data->row_top = GUIHBox_new();
    data->row_bot = GUIHBox_new();
    data->lbl_p1 = GUILabel_new();
    data->lbl_p2 = GUILabel_new();
    data->lbl_p3 = GUILabel_new();
    data->lbl_p4 = GUILabel_new();

    GUILabel_upside_down_en(data->lbl_p1, 1);
    GUILabel_upside_down_en(data->lbl_p2, 1);
    GUI_ADD_CHILD(data->row_top, data->lbl_p1);
    GUI_ADD_CHILD(data->row_top, data->lbl_p2);
    GUI_ADD_CHILD(data->row_bot, data->lbl_p3);
    GUI_ADD_CHILD(data->row_bot, data->lbl_p4);
    GUI_ADD_CHILD(data->root_vbox, data->row_top);
    GUI_ADD_CHILD(data->root_vbox, data->row_bot);
    GUI_SET_SIZE(data->root_vbox, 128, 64);
    GUI_SET_PADDING(data->root_vbox, 5);
    GUI_SET_SPACING(data->row_top, 20);
    GUI_SET_SPACING(data->row_bot, 20);
    GUI_UPDATE_LAYOUT(data->root_vbox);
    current_page.data = data;
    current_page.handle_input = MainPage_handle_input;
    current_page.exit = MainPage_delete;
    MainPage_update();
}
void ViewModel_init() {
    GameSettings_init();
    Game_init();
    GUIRenderer_init();
    MainPage_set();
}