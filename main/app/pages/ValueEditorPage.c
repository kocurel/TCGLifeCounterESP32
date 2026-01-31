#include "ValueEditorPage.h"

#include <stdio.h>
#include <string.h>

#include "AudioManager.h"
#include "GUIFramework.h"
#include "app/PageManager.h"
#include "model/Game.h"

// Maximum value for display clamping (+/- 999999)
#define MAX_DISPLAY_VAL 999999

// --- Context Definition ---
typedef struct {
    // UI Widgets
    GUILabel title_lbl;
    GUILabel subtitle_lbl;
    GUIVBox title_box;

    GUILabel old_value_lbl;
    GUILabel difference_lbl;
    GUILabel new_value_lbl;
    GUILabel scale_lbl;

    // Logic State
    uint8_t active_value_id;  // Current value being edited (0=HP, etc.)
    int32_t entry_value;      // The starting value
    int32_t current_value;    // The current result

    bool is_initialized;
    void (*on_complete_callback)(int32_t result);

    char text_buffer[32];
} ValueEditorContext;

// --- Persistent State ---
// Static array to store current scale per value index.
// Survives page transitions.
static int32_t s_persistent_scales[NUMBER_OF_VALUES];

// --- Global Context Instance ---
static ValueEditorContext ctx = {.is_initialized = false};

// --- Helper Functions ---

static int32_t get_current_scale() {
    return s_persistent_scales[ctx.active_value_id];
}

static void update_scale_label() {
    int32_t scale = get_current_scale();

    if (scale >= 1000) {
        // Use "k" notation for large steps (1k, 10k, etc.)
        snprintf(ctx.text_buffer, sizeof(ctx.text_buffer), "x%ldk",
                 scale / 1000);
    } else {
        snprintf(ctx.text_buffer, sizeof(ctx.text_buffer), "x%ld", scale);
    }

    GUI_SET_TEXT(&ctx.scale_lbl, ctx.text_buffer);
}

static void ValueEditorPage_draw() {
    GUIRenderer_clear_buffer();

    // 1. Draw Static Header
    GUI_DRAW(&ctx.title_box);

    // 2. Format & Draw "Difference" Row
    int32_t delta = ctx.current_value - ctx.entry_value;
    snprintf(ctx.text_buffer, sizeof(ctx.text_buffer), "Change:%+8ld", delta);
    GUI_SET_TEXT(&ctx.difference_lbl, ctx.text_buffer);
    GUI_DRAW(&ctx.difference_lbl);

    // 3. Format & Draw "Result" Row
    snprintf(ctx.text_buffer, sizeof(ctx.text_buffer), "Result:%+8ld",
             ctx.current_value);
    GUI_SET_TEXT(&ctx.new_value_lbl, ctx.text_buffer);
    GUI_DRAW(&ctx.new_value_lbl);

    // 4. Draw Reference "Old Value"
    GUI_DRAW(&ctx.old_value_lbl);

    // 5. Draw Visual Separator line
    GUIRenderer_draw_horizontal_line(47);

    // 6. Draw Scale Indicator
    update_scale_label();
    GUI_DRAW(&ctx.scale_lbl);

    GUIRenderer_send_buffer();
}

static void ValueEditorPage_handle_input(ButtonCode button) {
    int32_t scale = get_current_scale();

    switch (button) {
        case BUTTON_CODE_UP:
            if (ctx.current_value + scale <= MAX_DISPLAY_VAL) {
                ctx.current_value += scale;
            } else {
                ctx.current_value = MAX_DISPLAY_VAL;
            }
            ValueEditorPage_draw();
            break;

        case BUTTON_CODE_DOWN:
            if (ctx.current_value - scale >= -MAX_DISPLAY_VAL) {
                ctx.current_value -= scale;
            } else {
                ctx.current_value = -MAX_DISPLAY_VAL;
            }
            ValueEditorPage_draw();
            break;

        case BUTTON_CODE_LEFT:
            // Increase Step Size (x10) for this specific value_id
            if (s_persistent_scales[ctx.active_value_id] < 100000) {
                s_persistent_scales[ctx.active_value_id] *= 10;
                ValueEditorPage_draw();
            }
            break;

        case BUTTON_CODE_RIGHT:
            // Decrease Step Size (/10) for this specific value_id
            if (s_persistent_scales[ctx.active_value_id] > 1) {
                s_persistent_scales[ctx.active_value_id] /= 10;
                ValueEditorPage_draw();
            }
            break;

        case BUTTON_CODE_ACCEPT:
            AudioManager_play_sound(SOUND_UI_SELECT);
            if (ctx.on_complete_callback) {
                ctx.on_complete_callback(ctx.current_value);
            }
            break;

        case BUTTON_CODE_CANCEL:
            AudioManager_play_sound(SOUND_UI_CANCEL);
            if (ctx.on_complete_callback) {
                ctx.on_complete_callback(ctx.entry_value);
            }
            break;

        default:
            break;
    }
}

void ValueEditorPage_enter(const char* title, const char* subtitle,
                           uint8_t value_id, int32_t value,
                           void (*callback)(int32_t new_value)) {
    // 1. One-time Initialization of Widgets and Persistent State
    if (!ctx.is_initialized) {
        // Default all scales to 1 on first boot
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

        GUI_SET_POS(&ctx.scale_lbl, 90, 34);
        GUI_SET_SIZE(&ctx.scale_lbl, 38, 8);
        GUILabel_set_alignment(&ctx.scale_lbl, GUI_ALIGMNENT_RIGHT);

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

    // 2. Set Context for this Session
    ctx.active_value_id = value_id;  // Store ID to access persistent scale
    ctx.entry_value = value;
    ctx.current_value = value;
    ctx.on_complete_callback = callback;

    // 3. Set Static Text for "Old Value"
    snprintf(ctx.text_buffer, sizeof(ctx.text_buffer), "Start:%8ld",
             ctx.entry_value);
    GUI_SET_TEXT(&ctx.old_value_lbl, ctx.text_buffer);

    // 4. Register Page and Switch
    Page new_page = {0};
    new_page.handle_input = ValueEditorPage_handle_input;
    PageManager_switch_page(&new_page);

    // 5. Initial Draw
    ValueEditorPage_draw();
}