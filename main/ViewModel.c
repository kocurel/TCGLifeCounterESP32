#include "ViewModel.h"

#include <memory.h>
#include <stdio.h>

#include "GUIFramework.h"
#include "Game.h"

typedef struct {
    int selected_index;
    GUIComponent* focused_component;
    GUIVBox root_vbox;
    GUIHBox row_top;
    GUIHBox row_bot;
    GUILabel lbl_p1;
    GUILabel lbl_p2;
    GUILabel lbl_p3;
    GUILabel lbl_p4;
} MainPageData;

static MainPageData main_page = {0};

typedef struct Page Page;
struct Page {
    void (*handle_input)(ButtonCode);
    void (*exit)();
    void* data;
};

static Page current_page = {0};

// --- Forward Declarations ---
static void MainPage_update(void);

// --- Input Handling Logic (Unchanged) ---
void ViewModel_handle_input(ButtonCode button) {
    if (current_page.handle_input == NULL) {
        return;
    }
    current_page.handle_input(button);
}

// --- Page Logic ---

static void MainPage_exit() {
    // Static allocation: No free() needed!
    // We just null out the current page pointer.
    current_page.data = NULL;
    current_page.handle_input = NULL;
    current_page.exit = NULL;
}

static void MainPage_update() {
    MainPageData* data = (MainPageData*)current_page.data;
    if (data == NULL) return;

    char str[16];  // Stack buffer for formatting

    // Update Text from Game Model
    snprintf(str, sizeof(str), "%ld", Game_get_value(0, 0));
    GUI_SET_TEXT(&data->lbl_p1, str);

    snprintf(str, sizeof(str), "%ld", Game_get_value(1, 0));
    GUI_SET_TEXT(&data->lbl_p2, str);

    snprintf(str, sizeof(str), "%ld", Game_get_value(2, 0));
    GUI_SET_TEXT(&data->lbl_p3, str);

    snprintf(str, sizeof(str), "%ld", Game_get_value(3, 0));
    GUI_SET_TEXT(&data->lbl_p4, str);

    // Draw
    GUIRenderer_clear_buffer();
    GUI_UPDATE_LAYOUT(&data->root_vbox);  // Recalculate positions
    GUI_DRAW(&data->root_vbox);

    if (data->focused_component != NULL) {
        uint8_t x, y, w, h;

        // Get dimensions of the currently focused object directly
        GUIComponent_get_xywh(data->focused_component, &x, &y, &w, &h);

        // Draw frame around it
        GUIRenderer_draw_frame(x, y, w, h);
    }
    GUIRenderer_send_buffer();
}

static void MainPage_handle_input(ButtonCode button) {
    MainPageData* data = (MainPageData*)current_page.data;
    if (data == NULL || data->focused_component == NULL) return;

    GUIComponent* next_focus = NULL;

    switch (button) {
        case BUTTON_CODE_UP:
            next_focus = data->focused_component->nav_up;
            break;
        case BUTTON_CODE_DOWN:
            next_focus = data->focused_component->nav_down;
            break;
        case BUTTON_CODE_LEFT:
            next_focus = data->focused_component->nav_left;
            break;
        case BUTTON_CODE_RIGHT:
            next_focus = data->focused_component->nav_right;
            break;

        case BUTTON_CODE_ACCEPT:
            // Handle clicking the focused item
            // e.g., if (data->focused_component ==
            // (GUIComponent*)&data->lbl_p1) ...
            break;

        default:
            break;
    }

    // Only update if a valid neighbor exists in that direction
    if (next_focus != NULL) {
        data->focused_component = next_focus;
        MainPage_update();  // Redraw to show new selection
    }
}

// --- Initialization (Replaces MainPage_set) ---
static void MainPage_enter() {
    // 1. Reset Data (Optional but good for safety)

    // 2. Set Default State
    main_page.selected_index = 1;

    // 3. Initialize Components (Using the static _init functions)
    // We pass the address of the embedded structs using '&'
    GUIVBox_init(&main_page.root_vbox);
    GUIHBox_init(&main_page.row_top);
    GUIHBox_init(&main_page.row_bot);

    // Note: Assuming empty text to start, update() will fill it
    GUILabel_init(&main_page.lbl_p1, "");
    GUILabel_init(&main_page.lbl_p2, "");
    GUILabel_init(&main_page.lbl_p3, "");
    GUILabel_init(&main_page.lbl_p4, "");

    // Link Top Row (P1 <-> P2)
    GUI_LINK_HORIZONTAL(&main_page.lbl_p1, &main_page.lbl_p2);

    // Link Bottom Row (P3 <-> P4)
    GUI_LINK_HORIZONTAL(&main_page.lbl_p3, &main_page.lbl_p4);

    // Link Columns (P1 <-> P3) and (P2 <-> P4)
    GUI_LINK_VERTICAL(&main_page.lbl_p1, &main_page.lbl_p3);
    GUI_LINK_VERTICAL(&main_page.lbl_p2, &main_page.lbl_p4);

    // Set initial focus
    main_page.focused_component = (GUIComponent*)&main_page.lbl_p1;

    // 4. Configure Properties
    GUILabel_upside_down_en(&main_page.lbl_p1, true);
    GUILabel_upside_down_en(&main_page.lbl_p2, true);

    GUI_ADD_CHILDREN(&main_page.row_top, &main_page.lbl_p1, &main_page.lbl_p2);

    GUI_ADD_CHILDREN(&main_page.row_bot, &main_page.lbl_p3, &main_page.lbl_p4);

    GUI_ADD_CHILDREN(&main_page.root_vbox, &main_page.row_top,
                     &main_page.row_bot);

    GUI_SET_SIZE(&main_page.root_vbox, 128, 64);
    GUI_SET_PADDING(&main_page.root_vbox, 5);
    GUI_SET_SPACING(&main_page.row_top, 20);
    GUI_SET_SPACING(&main_page.row_bot, 20);

    GUI_UPDATE_LAYOUT(&main_page.root_vbox);

    current_page.data = &main_page;
    current_page.handle_input = MainPage_handle_input;
    current_page.exit = MainPage_exit;

    MainPage_update();
}

void ViewModel_init() {
    Game_init();
    GUIRenderer_init();
    MainPage_enter();
}