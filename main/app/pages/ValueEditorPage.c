#include "ValueEditorPage.h"

#include "GUIFramework.h"
#include "app/PageManager.h"

static GUILabel title_lbl = {0};
static GUILabel subtitle_lbl = {0};
static GUIVBox title_box = {0};

static GUILabel old_value_lbl = {0};
static GUILabel difference_lbl = {0};
static GUILabel new_value_lbl = {0};

static GUILabel scale_lbl = {0};

static int32_t current_scale = 1;

static int32_t entry_value = 0;
static int32_t current_value = 0;
static bool is_initialized = false;

static void (*callback_function)(int32_t new_value);
static char buffer[24];

void ValueEditorPage_update() {
    GUIRenderer_clear_buffer();
    snprintf(buffer, 24, "Change:%+8ld", current_value - entry_value);
    GUI_SET_TEXT(&difference_lbl, buffer);
    snprintf(buffer, 24, "Result:%8ld", current_value);
    GUI_SET_TEXT(&new_value_lbl, buffer);
    GUI_DRAW(&title_box);
    GUIRenderer_draw_horizontal_line(47);
    GUI_DRAW(&difference_lbl);
    GUI_DRAW(&new_value_lbl);
    GUI_DRAW(&old_value_lbl);
    GUIRenderer_send_buffer();
}

void ValueEditorPage_handle_input(ButtonCode button) {
    switch (button) {
        case BUTTON_CODE_UP:
            current_value += current_scale;
            if (current_value > 999999) {
                current_value = 999999;
            }
            ValueEditorPage_update();
            break;
        case BUTTON_CODE_DOWN:
            current_value -= current_scale;
            if (current_value < -999999) {
                current_value = -999999;
            }
            ValueEditorPage_update();
            break;
        case BUTTON_CODE_LEFT:
            break;
        case BUTTON_CODE_RIGHT:
            break;

        default:
            break;
    }
}

void ValueEditorPage_enter(char* title, char* subtitle, int32_t value,
                           void (*callback)(int32_t new_value)) {
    if (!is_initialized) {
        GUILabel_init(&title_lbl, title);
        GUILabel_init(&subtitle_lbl, subtitle);

        GUILabel_init(&old_value_lbl, "");
        GUILabel_init(&difference_lbl, "");
        GUILabel_init(&scale_lbl, "");
        GUILabel_init(&new_value_lbl, "");
        GUIVBox_init(&title_box);

        GUI_SET_FONT_SIZE(&title_lbl, 6);
        GUI_SET_FONT_SIZE(&subtitle_lbl, 6);
        GUI_SET_FONT_SIZE(&old_value_lbl, 7);
        GUI_SET_FONT_SIZE(&difference_lbl, 7);
        GUI_SET_FONT_SIZE(&scale_lbl, 7);
        GUI_SET_FONT_SIZE(&new_value_lbl, 7);

        GUILabel_set_alignment(&old_value_lbl, GUI_ALIGMNENT_RIGHT);
        GUILabel_set_alignment(&difference_lbl, GUI_ALIGMNENT_RIGHT);
        GUILabel_set_alignment(&new_value_lbl, GUI_ALIGMNENT_RIGHT);

        is_initialized = true;
        GUI_ADD_CHILDREN(&title_box, &title_lbl, &subtitle_lbl);
        GUI_SET_SIZE(&title_box, 128, 20);
        GUI_SET_PADDING(&title_box, 2);
        GUI_SET_SPACING(&title_box, 2);
        GUI_UPDATE_LAYOUT(&title_box);

        GUI_SET_POS(&old_value_lbl, 0, 22);
        GUI_SET_SIZE(&old_value_lbl, 104, 8);

        GUI_SET_POS(&difference_lbl, 0, 34);
        GUI_SET_SIZE(&difference_lbl, 104, 8);

        GUI_SET_POS(&new_value_lbl, 0, 50);
        GUI_SET_SIZE(&new_value_lbl, 104, 8);
    }
    entry_value = value;
    current_value = value;
    callback_function = callback;

    snprintf(buffer, 24, "Start:%8ld", entry_value);
    GUI_SET_TEXT(&old_value_lbl, buffer);

    Page new_page = {0};
    new_page.handle_input = ValueEditorPage_handle_input;
    PageManager_switch_page(&new_page);
    ValueEditorPage_update();
}