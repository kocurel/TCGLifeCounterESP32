#include "ConfirmPage.h"

#include <string.h>

#include "GUIFramework.h"
#include "GUIRenderer.h"
#include "MainPage.h"
#include "app/PageManager.h"

// Statyczne komponenty strony
static GUILabel lbl_message;
static GUILabel lbl_yes;
static GUILabel lbl_no;
static ConfirmCallback confirm_action = NULL;
static bool is_initialized = false;

static void ConfirmPage_init_components() {
    if (is_initialized) return;

    // Główna wiadomość
    GUILabel_init(&lbl_message, "");
    GUILabel_set_font_size(&lbl_message, 9);
    GUILabel_set_alignment(&lbl_message, GUI_ALIGMNENT_CENTER);
    GUI_SET_POS(&lbl_message, 0, 20);
    GUI_SET_SIZE(&lbl_message, 128, 14);

    // Przycisk TAK (lewa strona)
    GUILabel_init(&lbl_yes, "[OK] Yes");
    GUILabel_set_font_size(&lbl_yes, 7);
    GUILabel_set_alignment(&lbl_yes, GUI_ALIGMNENT_RIGHT);
    GUI_SET_POS(&lbl_yes, 63, 54);
    GUI_SET_SIZE(&lbl_yes, 60, 10);

    // Przycisk NIE (prawa strona)
    GUILabel_init(&lbl_no, "No [X]");
    GUILabel_set_font_size(&lbl_no, 7);
    GUILabel_set_alignment(&lbl_no, GUI_ALIGMNENT_LEFT);
    GUI_SET_POS(&lbl_no, 5, 54);
    GUI_SET_SIZE(&lbl_no, 60, 10);

    is_initialized = true;
}

void ConfirmPage_draw() {
    GUIRenderer_clear_buffer();

    // Rysowanie etykiet (one same zajmą się centrowaniem/wyrównaniem)
    GUI_DRAW(&lbl_message);
    GUI_DRAW(&lbl_yes);
    GUI_DRAW(&lbl_no);

    // Linia oddzielająca pasek akcji
    GUIRenderer_draw_line(0, 50, 128, 50);

    GUIRenderer_send_buffer();
}

static void ConfirmPage_handle_input(ButtonCode button) {
    switch (button) {
        case BUTTON_CODE_ACCEPT:  // OK / YES
            if (confirm_action) confirm_action();
            MainPage_enter();
            break;

        case BUTTON_CODE_CANCEL:  // X / NO
            MainPage_enter();
            break;

        default:
            break;
    }
}

void ConfirmPage_enter(const char* message, ConfirmCallback on_confirm) {
    ConfirmPage_init_components();

    // Ustawienie dynamicznej treści
    GUI_SET_TEXT(&lbl_message, message);
    confirm_action = on_confirm;

    Page page = {.handle_input = ConfirmPage_handle_input, .exit = NULL};
    PageManager_switch_page(&page);

    ConfirmPage_draw();
}