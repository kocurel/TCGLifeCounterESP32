#include "ValueEditorPage.h"

#include <stdio.h>
#include <string.h>

#include "AudioManager.h"
#include "GUIFramework.h"
#include "app/PageManager.h"
#include "model/Game.h"

#define MAX_DISPLAY_VAL 999999

typedef struct {
    GUILabel title_lbl;
    GUILabel subtitle_lbl;
    GUIVBox title_box;

    GUILabel old_value_lbl;
    GUILabel difference_lbl;
    GUILabel new_value_lbl;
    GUILabel scale_lbl;

    uint8_t active_value_id;
    int32_t entry_value;
    int32_t current_value;

    bool is_initialized;
    void (*on_complete_callback)(int32_t result);

    char text_buffer[32];
} ValueEditorContext;

/* Persists increment magnitudes across page sessions */
static int32_t s_persistent_scales[NUMBER_OF_VALUES];

static ValueEditorContext ctx = {.is_initialized = false};

static int32_t get_current_scale() {
    return s_persistent_scales[ctx.active_value_id];
}

/**
 * Updates scale indicator with optional kilo-suffix formatting
 */
static void update_scale_label() {
    int32_t scale = get_current_scale();

    if (scale >= 1000) {
        snprintf(ctx.text_buffer, sizeof(ctx.text_buffer), "x%ldk",
                 scale / 1000);
    } else {
        snprintf(ctx.text_buffer, sizeof(ctx.text_buffer), "x%ld", scale);
    }

    GUI_SET_TEXT(&ctx.scale_lbl, ctx.text_buffer);
}

static void ValueEditorPage_draw() {
    GUIRenderer_clear_buffer();

    GUI_DRAW(&ctx.title_box);

    const int32_t delta = ctx.current_value - ctx.entry_value;

    snprintf(ctx.text_buffer, sizeof(ctx.text_buffer), "Change:%+8ld", delta);
    GUI_SET_TEXT(&ctx.difference_lbl, ctx.text_buffer);
    GUI_DRAW(&ctx.difference_lbl);

    snprintf(ctx.text_buffer, sizeof(ctx.text_buffer), "Result:%+8ld",
             ctx.current_value);
    GUI_SET_TEXT(&ctx.new_value_lbl, ctx.text_buffer);
    GUI_DRAW(&ctx.new_value_lbl);

    GUI_DRAW(&ctx.old_value_lbl);

    GUIRenderer_draw_horizontal_line(47);
    update_scale_label();
    GUI_DRAW(&ctx.scale_lbl);

    GUIRenderer_send_buffer();
}

static void ValueEditorPage_handle_input(ButtonCode button) {
    const int32_t scale = get_current_scale();

    switch (button) {
        case BUTTON_CODE_UP:
            if (ctx.current_value + scale <= MAX_DISPLAY_VAL) {
                ctx.current_value += scale;
            } else {
                ctx.current_value = MAX_DISPLAY_VAL;
            }
            break;

        case BUTTON_CODE_DOWN:
            if (ctx.current_value - scale >= -MAX_DISPLAY_VAL) {
                ctx.current_value -= scale;
            } else {
                ctx.current_value = -MAX_DISPLAY_VAL;
            }
            break;

        case BUTTON_CODE_LEFT:
            /* Increase increment magnitude */
            if (s_persistent_scales[ctx.active_value_id] < 100000) {
                s_persistent_scales[ctx.active_value_id] *= 10;
            }
            break;

        case BUTTON_CODE_RIGHT:
            /* Decrease increment magnitude */
            if (s_persistent_scales[ctx.active_value_id] > 1) {
                s_persistent_scales[ctx.active_value_id] /= 10;
            }
            break;

        case BUTTON_CODE_ACCEPT:
            AudioManager_play_sound(SOUND_UI_SELECT);
            if (ctx.on_complete_callback) {
                ctx.on_complete_callback(ctx.current_value);
            }
            return;

        case BUTTON_CODE_CANCEL:
            AudioManager_play_sound(SOUND_UI_CANCEL);
            if (ctx.on_complete_callback) {
                ctx.on_complete_callback(ctx.entry_value);
            }
            return;

        default:
            break;
    }
    ValueEditorPage_draw();
}

void ValueEditorPage_enter(const char* title, const char* subtitle,
                           uint8_t value_id, int32_t value,
                           void (*callback)(int32_t new_value)) {
    /* Lazy initialization of UI components and scale defaults */
    if (!ctx.is_initialized) {
        for (int i = 0; i < NUMBER_OF_VALUES; i++) {
            s_persistent_scales[i] = 1;
        }

        GUILabel_init(&ctx.title_lbl, title);
        GUILabel_init(&ctx.subtitle_lbl, subtitle);
        GUILabel_init(&ctx.old_value_lbl, "");
        GUILabel_init(&ctx.difference_lbl, "");
        GUILabel_init(&ctx.scale_lbl, "");
        GUILabel_init(&ctx.new_value_lbl, "");
        GUIVBox_init(&ctx.title_box);

        GUI_SET_FONT_SIZE(&ctx.title_lbl, 6);
        GUI_SET_FONT_SIZE(&ctx.subtitle_lbl, 6);
        GUI_SET_FONT_SIZE(&ctx.old_value_lbl, 7);
        GUI_SET_FONT_SIZE(&ctx.difference_lbl, 7);
        GUI_SET_FONT_SIZE(&ctx.scale_lbl, 6);
        GUI_SET_FONT_SIZE(&ctx.new_value_lbl, 7);

        GUILabel_set_alignment(&ctx.old_value_lbl, GUI_ALIGMNENT_RIGHT);
        GUILabel_set_alignment(&ctx.difference_lbl, GUI_ALIGMNENT_RIGHT);
        GUILabel_set_alignment(&ctx.new_value_lbl, GUI_ALIGMNENT_RIGHT);
        GUILabel_set_alignment(&ctx.scale_lbl, GUI_ALIGMNENT_RIGHT);

        GUI_SET_POS(&ctx.scale_lbl, 90, 34);
        GUI_SET_SIZE(&ctx.scale_lbl, 38, 8);

        GUI_ADD_CHILDREN(&ctx.title_box, &ctx.title_lbl, &ctx.subtitle_lbl);
        GUI_SET_SIZE(&ctx.title_box, 128, 20);
        GUI_SET_PADDING(&ctx.title_box, 2);
        GUI_SET_SPACING(&ctx.title_box, 2);
        GUI_UPDATE_LAYOUT(&ctx.title_box);

        GUI_SET_POS(&ctx.old_value_lbl, 0, 22);
        GUI_SET_SIZE(&ctx.old_value_lbl, 96, 8);
        GUI_SET_POS(&ctx.difference_lbl, 0, 34);
        GUI_SET_SIZE(&ctx.difference_lbl, 96, 8);
        GUI_SET_POS(&ctx.new_value_lbl, 0, 50);
        GUI_SET_SIZE(&ctx.new_value_lbl, 96, 8);

        ctx.is_initialized = true;
    } else {
        GUI_SET_TEXT(&ctx.title_lbl, title);
        GUI_SET_TEXT(&ctx.subtitle_lbl, subtitle);
    }

    ctx.active_value_id = value_id;
    ctx.entry_value = value;
    ctx.current_value = value;
    ctx.on_complete_callback = callback;

    snprintf(ctx.text_buffer, sizeof(ctx.text_buffer), "Start:%8ld",
             ctx.entry_value);
    GUI_SET_TEXT(&ctx.old_value_lbl, ctx.text_buffer);

    Page new_page = {.handle_input = ValueEditorPage_handle_input};
    PageManager_switch_page(&new_page);

    ValueEditorPage_draw();
}