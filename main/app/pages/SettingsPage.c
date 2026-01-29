#include "SettingsPage.h"

#include <stdio.h>

#include "AudioManager.h"
#include "GUIFramework.h"
#include "MenuPage.h"
#include "app/PageManager.h"

// --- Forward Declarations for Stubs ---
void GameSettingsPage_enter();
void DeviceSettingsPage_enter();

// --- Data Structure ---
typedef struct {
    GUIComponent* focused_component;
    GUIVBox root_vbox;
    GUILabel lbl_game_settings;
    GUILabel lbl_device_settings;
} SettingsPageData;

static SettingsPageData settings_page = {0};
static bool is_initialized = false;

static void SettingsPage_update() {
    GUIRenderer_clear_buffer();

    // 1. Draw the container (handles children automatically)
    GUI_DRAW(&settings_page.root_vbox);

    // 2. Draw Focus Frame around the currently selected component
    if (settings_page.focused_component != NULL) {
        uint8_t x, y, w, h;
        GUIComponent_get_xywh(settings_page.focused_component, &x, &y, &w, &h);

        // Draw a rectangle around the selected label
        GUIRenderer_draw_frame(x, y, w, h);
    }

    GUIRenderer_send_buffer();
}

static void SettingsPage_handle_input(ButtonCode button) {
    GUIComponent* next_focus = NULL;

    switch (button) {
        case BUTTON_CODE_UP:
            next_focus = settings_page.focused_component->nav_up;
            break;
        case BUTTON_CODE_DOWN:
            next_focus = settings_page.focused_component->nav_down;
            break;

        case BUTTON_CODE_ACCEPT:
            AudioManager_play_sound(SOUND_UI_SELECT);

            // Check pointer equality to determine which label was selected
            if (settings_page.focused_component ==
                (GUIComponent*)&settings_page.lbl_game_settings) {
                GameSettingsPage_enter();
            } else if (settings_page.focused_component ==
                       (GUIComponent*)&settings_page.lbl_device_settings) {
                DeviceSettingsPage_enter();
            }
            return;  // Return immediately to prevent redraw on old page

        case BUTTON_CODE_CANCEL:
            AudioManager_play_sound(SOUND_UI_CANCEL);
            MenuPage_enter();
            return;

        default:
            break;
    }

    if (next_focus != NULL) {
        AudioManager_play_sound(SOUND_UI_MOVE);
        settings_page.focused_component = next_focus;
        SettingsPage_update();
    }
}

void SettingsPage_enter() {
    if (!is_initialized) {
        // 1. Init Components
        GUIVBox_init(&settings_page.root_vbox);
        GUILabel_init(&settings_page.lbl_game_settings, "Game Settings");
        GUILabel_init(&settings_page.lbl_device_settings, "Device Settings");

        // 2. Setup Container Layout
        GUI_SET_POS(&settings_page.root_vbox, 0, 10);
        GUI_SET_SIZE(&settings_page.root_vbox, 128, 54);
        GUI_SET_SPACING(&settings_page.root_vbox, 12);  // Gap between labels

        // 3. Setup Labels (Center aligned text)
        GUI_SET_FONT_SIZE(&settings_page.lbl_game_settings, 7);
        GUILabel_set_alignment(&settings_page.lbl_game_settings,
                               GUI_ALIGMNENT_CENTER);

        GUI_SET_FONT_SIZE(&settings_page.lbl_device_settings, 7);
        GUILabel_set_alignment(&settings_page.lbl_device_settings,
                               GUI_ALIGMNENT_CENTER);

        // 4. Add to VBox
        GUI_ADD_CHILDREN(&settings_page.root_vbox,
                         &settings_page.lbl_game_settings,
                         &settings_page.lbl_device_settings);

        // 5. Calculate Layout (Positions X/Y for children)
        GUI_UPDATE_LAYOUT(&settings_page.root_vbox);

        // 6. Navigation Linking
        // Standard Vertical Link (Game -> Down -> Device)
        GUI_LINK_VERTICAL(&settings_page.lbl_game_settings,
                          &settings_page.lbl_device_settings);

        // 7. Manual Wrap-Around Logic
        // Device -> Down -> Game
        settings_page.lbl_device_settings.base.nav_down =
            (GUIComponent*)&settings_page.lbl_game_settings;
        // Game -> Up -> Device
        settings_page.lbl_game_settings.base.nav_up =
            (GUIComponent*)&settings_page.lbl_device_settings;

        is_initialized = true;
    }

    // Reset focus to top when entering
    settings_page.focused_component =
        (GUIComponent*)&settings_page.lbl_game_settings;

    Page new_page = {.handle_input = SettingsPage_handle_input, .exit = NULL};
    PageManager_switch_page(&new_page);
    SettingsPage_update();
}

// --- Stubs ---
void GameSettingsPage_enter() {
    printf("[STUB] Entering Game Settings\n");
    SettingsPage_enter();  // Go back for now
}

void DeviceSettingsPage_enter() {
    printf("[STUB] Entering Device Settings\n");
    SettingsPage_enter();  // Go back for now
}