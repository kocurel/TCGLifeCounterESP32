#include "ConfirmPage.h"

#include <stddef.h>
#include <string.h>

#include "GUIFramework.h"
#include "GUIRenderer.h"
#include "app/PageManager.h"

/* --- Private State --- */
static GUILabel lbl_message;
static GUILabel lbl_yes;
static GUILabel lbl_no;

static ConfirmCallback confirm_action = NULL;
static void (*cancel_target)(void) =
    NULL;  // Page to return to if "No" is pressed
static bool is_initialized = false;

/* --- Component Initialization --- */
static void ConfirmPage_init_components() {
    if (is_initialized) return;

    // Main prompt message
    GUILabel_init(&lbl_message, "");
    GUILabel_set_font_size(&lbl_message, 9);
    GUILabel_set_alignment(&lbl_message, GUI_ALIGMNENT_CENTER);
    GUI_SET_POS(&lbl_message, 0, 20);
    GUI_SET_SIZE(&lbl_message, 128, 14);

    // YES option (ACCEPT/OK)
    GUILabel_init(&lbl_yes, "[OK] Yes");
    GUILabel_set_font_size(&lbl_yes, 7);
    GUILabel_set_alignment(&lbl_yes, GUI_ALIGMNENT_RIGHT);
    GUI_SET_POS(&lbl_yes, 63, 54);
    GUI_SET_SIZE(&lbl_yes, 60, 10);

    // NO option (CANCEL/X)
    GUILabel_init(&lbl_no, "No [X]");
    GUILabel_set_font_size(&lbl_no, 7);
    GUILabel_set_alignment(&lbl_no, GUI_ALIGMNENT_LEFT);
    GUI_SET_POS(&lbl_no, 5, 54);
    GUI_SET_SIZE(&lbl_no, 60, 10);

    is_initialized = true;
}

/* --- Drawing --- */
static void ConfirmPage_draw() {
    GUIRenderer_clear_buffer();

    GUI_DRAW(&lbl_message);
    GUI_DRAW(&lbl_yes);
    GUI_DRAW(&lbl_no);

    GUIRenderer_draw_line(0, 50, 128, 50);

    GUIRenderer_send_buffer();
}

/* --- Input Handling --- */
static void ConfirmPage_handle_input(ButtonCode button) {
    switch (button) {
        case BUTTON_CODE_ACCEPT:
            // USER RESPONSIBILITY: The callback must handle the page
            // transition!
            if (confirm_action) {
                confirm_action();
            }
            break;

        case BUTTON_CODE_CANCEL:
            // Return to the previous context provided during 'enter'
            if (cancel_target) {
                cancel_target();
            }
            break;

        default:
            break;
    }
}

/* --- Page Lifecycle --- */

/**
 * Enters the confirmation page.
 * @param message The text to display.
 * @param on_confirm Callback executed if "Yes" is selected (must handle its own
 * page switch).
 * @param on_cancel The page-entry function to call if "No" is pressed.
 */
void ConfirmPage_enter(const char* message, ConfirmCallback on_confirm,
                       void (*on_cancel)(void)) {
    ConfirmPage_init_components();

    GUI_SET_TEXT(&lbl_message, message);
    confirm_action = on_confirm;
    cancel_target = on_cancel;

    Page page = {.handle_input = ConfirmPage_handle_input};

    PageManager_switch_page(&page);
    ConfirmPage_draw();
}