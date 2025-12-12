#include "ViewModel.h"

#include <memory.h>
#include <stdio.h>

#include "GUIFramework.h"
#include "Game.h"
#include "Utils.h"

// --- 1. Define the Data Structure for the Page ---
typedef struct {
    int selected_index;
    GUIVBox root_vbox;
    GUIHBox row_top;
    GUIHBox row_bot;
    GUILabel lbl_p1;
    GUILabel lbl_p2;
    GUILabel lbl_p3;
    GUILabel lbl_p4;
} MainPageData;

// --- 2. Allocate the Page Statically ---
// This reserves the memory for the entire page logic and UI tree.
static MainPageData main_page;

// --- Page Interface Wrapper ---
typedef struct Page Page;
struct Page {
    void (*handle_input)(ButtonCode);
    void (*exit)();
    void* data;  // Points to &main_page
};

static Page current_page = {
    .handle_input = NULL,
    .exit = NULL,
    .data = NULL,
};

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

    // Draw Selection Frame
    int selected_label_id = data->selected_index;
    uint8_t x, y, w, h;

    // Note: data->lbl_p1 is a struct, so we pass its address (&)
    if (selected_label_id == 1)
        GUIComponent_get_xywh((GUIComponent*)&data->lbl_p1, &x, &y, &w, &h);
    else if (selected_label_id == 2)
        GUIComponent_get_xywh((GUIComponent*)&data->lbl_p2, &x, &y, &w, &h);
    else if (selected_label_id == 3)
        GUIComponent_get_xywh((GUIComponent*)&data->lbl_p3, &x, &y, &w, &h);
    else if (selected_label_id == 4)
        GUIComponent_get_xywh((GUIComponent*)&data->lbl_p4, &x, &y, &w, &h);

    GUIRenderer_draw_frame(x, y, w, h);
    GUIRenderer_send_buffer();
}

static void MainPage_handle_input(ButtonCode button) {
    MainPageData* data = (MainPageData*)current_page.data;
    if (data == NULL) return;

    int selected_label = data->selected_index;

    switch (button) {
        case BUTTON_CODE_ACCEPT:
            break;
        case BUTTON_CODE_CANCEL:
            break;
        case BUTTON_CODE_MENU:
            break;
        case BUTTON_CODE_SET:
            break;
        // Navigation Logic
        case BUTTON_CODE_LEFT:
            if (selected_label == 2) data->selected_index = 1;
            if (selected_label == 4) data->selected_index = 3;
            break;
        case BUTTON_CODE_RIGHT:
            if (selected_label == 1) data->selected_index = 2;
            if (selected_label == 3) data->selected_index = 4;
            break;
        case BUTTON_CODE_UP:
            if (selected_label == 3) data->selected_index = 1;
            if (selected_label == 4) data->selected_index = 2;
            break;
        case BUTTON_CODE_DOWN:
            if (selected_label == 1) data->selected_index = 3;
            if (selected_label == 2) data->selected_index = 4;
            break;
        default:
            break;
    }
    MainPage_update();
}

// --- Initialization (Replaces MainPage_set) ---
static void MainPage_enter() {
    // 1. Reset Data (Optional but good for safety)
    memset(&main_page, 0, sizeof(MainPageData));

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

    // 4. Configure Properties
    GUILabel_upside_down_en(&main_page.lbl_p1, true);
    GUILabel_upside_down_en(&main_page.lbl_p2, true);

    // 5. Build the Tree (Composition)
    // Pass addresses of parents and children
    GUI_ADD_CHILD(&main_page.row_top, &main_page.lbl_p1);
    GUI_ADD_CHILD(&main_page.row_top, &main_page.lbl_p2);

    GUI_ADD_CHILD(&main_page.row_bot, &main_page.lbl_p3);
    GUI_ADD_CHILD(&main_page.row_bot, &main_page.lbl_p4);

    GUI_ADD_CHILD(&main_page.root_vbox, &main_page.row_top);
    GUI_ADD_CHILD(&main_page.root_vbox, &main_page.row_bot);

    // 6. Layout Settings
    GUI_SET_SIZE(&main_page.root_vbox, 128, 64);
    GUI_SET_PADDING(&main_page.root_vbox, 5);
    GUI_SET_SPACING(&main_page.row_top, 20);
    GUI_SET_SPACING(&main_page.row_bot, 20);

    // 7. Initial Layout Calculation
    GUI_UPDATE_LAYOUT(&main_page.root_vbox);

    // 8. Assign to Current Page
    current_page.data = &main_page;  // Point to our static instance
    current_page.handle_input = MainPage_handle_input;
    current_page.exit = MainPage_exit;

    // 9. Initial Draw
    MainPage_update();
}

void ViewModel_init() {
    // Initialize Systems
    // GameSettings_init(); // Assuming these exist
    Game_init();
    GUIRenderer_init();

    // Start the App
    MainPage_enter();
}