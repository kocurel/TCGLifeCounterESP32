#include "ValueEditorPage.h"

#include <stdio.h>   // For snprintf
#include <string.h>  // For memset

#include "GUIFramework.h"
#include "app/PageManager.h"

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
    int32_t current_scale;  // 1, 10, 100, etc.
    int32_t entry_value;    // The starting value (e.g., 40)
    int32_t current_value;  // The current result (e.g., 45)

    bool is_initialized;
    void (*on_complete_callback)(int32_t result);

    char text_buffer[32];  // Increased slightly for safety
} ValueEditorContext;

// --- Global Context Instance ---
static ValueEditorContext ctx = {.current_scale = 1, .is_initialized = false};

// --- Helper Functions ---

static void update_scale_label() {
    if (ctx.current_scale >= 1000) {
        // Use "k" notation for large steps
        // 1000 -> "1k", 10000 -> "10k", 100000 -> "100k"
        snprintf(ctx.text_buffer, sizeof(ctx.text_buffer), "x%ldk",
                 ctx.current_scale / 1000);
    } else {
        // Use normal notation for small steps
        snprintf(ctx.text_buffer, sizeof(ctx.text_buffer), "x%ld",
                 ctx.current_scale);
    }

    GUI_SET_TEXT(&ctx.scale_lbl, ctx.text_buffer);
}

void ValueEditorPage_update() {
    GUIRenderer_clear_buffer();

    // 1. Draw Static Header
    GUI_DRAW(&ctx.title_box);

    // 2. Format & Draw "Difference" Row (Show signed delta: +5, -3)
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

    // 6. Draw Scale Indicator (Bottom right or custom position)
    update_scale_label();
    GUI_DRAW(&ctx.scale_lbl);

    GUIRenderer_send_buffer();
}

void ValueEditorPage_handle_input(ButtonCode button) {
    switch (button) {
        case BUTTON_CODE_UP:
            // Add scale, check clamp
            if (ctx.current_value + ctx.current_scale <= MAX_DISPLAY_VAL) {
                ctx.current_value += ctx.current_scale;
            } else {
                ctx.current_value = MAX_DISPLAY_VAL;
            }
            ValueEditorPage_update();
            break;

        case BUTTON_CODE_DOWN:
            // Subtract scale, check clamp (allowing negative)
            if (ctx.current_value - ctx.current_scale >= -MAX_DISPLAY_VAL) {
                ctx.current_value -= ctx.current_scale;
            } else {
                ctx.current_value = -MAX_DISPLAY_VAL;
            }
            ValueEditorPage_update();
            break;

        case BUTTON_CODE_LEFT:
            // Increase Step Size (x10) up to a limit (e.g., 1000)
            if (ctx.current_scale < 100000) {
                ctx.current_scale *= 10;
                ValueEditorPage_update();
            }
            break;

        case BUTTON_CODE_RIGHT:
            // Decrease Step Size (/10) down to 1
            if (ctx.current_scale > 1) {
                ctx.current_scale /= 10;
                ValueEditorPage_update();
            }
            break;

        case BUTTON_CODE_ACCEPT:  // Assuming you have an Accept/OK button
            if (ctx.on_complete_callback) {
                ctx.on_complete_callback(ctx.current_value);
            }
            break;

        // Optional: Cancel support (return original value)
        case BUTTON_CODE_CANCEL:
            if (ctx.on_complete_callback) {
                ctx.on_complete_callback(ctx.entry_value);
            }
            break;

        default:
            break;
    }
}

void ValueEditorPage_enter(const char* title, const char* subtitle,
                           int32_t value, void (*callback)(int32_t new_value)) {
    // 1. One-time Initialization of Widgets
    if (!ctx.is_initialized) {
        GUILabel_init(&ctx.title_lbl, title);
        GUILabel_init(&ctx.subtitle_lbl, subtitle);

        GUILabel_init(&ctx.old_value_lbl, "");
        GUILabel_init(&ctx.difference_lbl, "");
        GUILabel_init(&ctx.scale_lbl, "");  // Init scale label
        GUILabel_init(&ctx.new_value_lbl, "");
        GUIVBox_init(&ctx.title_box);

        GUI_SET_FONT_SIZE(&ctx.title_lbl, 6);
        GUI_SET_FONT_SIZE(&ctx.subtitle_lbl, 6);
        GUI_SET_FONT_SIZE(&ctx.old_value_lbl, 7);
        GUI_SET_FONT_SIZE(&ctx.difference_lbl, 7);
        GUI_SET_FONT_SIZE(&ctx.scale_lbl,
                          6);  // Slightly smaller for tool label
        GUI_SET_FONT_SIZE(&ctx.new_value_lbl, 7);

        GUILabel_set_alignment(&ctx.old_value_lbl, GUI_ALIGMNENT_RIGHT);
        GUILabel_set_alignment(&ctx.difference_lbl, GUI_ALIGMNENT_RIGHT);
        GUILabel_set_alignment(&ctx.new_value_lbl, GUI_ALIGMNENT_RIGHT);

        // Position Scale label (e.g., bottom right corner or near the number)
        GUI_SET_POS(&ctx.scale_lbl, 90, 34);
        GUI_SET_SIZE(&ctx.scale_lbl, 38, 8);
        GUILabel_set_alignment(&ctx.scale_lbl, GUI_ALIGMNENT_RIGHT);

        ctx.is_initialized = true;

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
    } else {
        // Just update dynamic text if reused
        GUI_SET_TEXT(&ctx.title_lbl, title);
        GUI_SET_TEXT(&ctx.subtitle_lbl, subtitle);
    }

    // 2. Reset Context State for this Session
    ctx.entry_value = value;
    ctx.current_value = value;
    ctx.current_scale = 1;  // Default to scale of 1
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
    ValueEditorPage_update();
}