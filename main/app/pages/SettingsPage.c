#include "SettingsPage.h"

#include <stdio.h>

#include "AudioManager.h"
#include "DeviceSettingsPage.h"
#include "GUIFramework.h"
#include "GameSettingsPage.h"
#include "MenuPage.h"
#include "app/PageManager.h"

/* --- Private Types --- */
typedef struct {
    GUIComponent* focused_component;
    GUIVBox root_vbox;
    GUILabel lbl_game_settings;
    GUILabel lbl_device_settings;
} SettingsPageData;

/* --- Private State --- */
static SettingsPageData settings_page = {0};
static bool is_initialized = false;

/* --- Drawing --- */

static void SettingsPage_draw() {
    GUIRenderer_clear_buffer();

    // 1. Render the main container and its children
    GUI_DRAW(&settings_page.root_vbox);

    // 2. Render Selection Frame
    if (settings_page.focused_component != NULL) {
        uint8_t x, y, w, h;
        GUIComponent_get_xywh(settings_page.focused_component, &x, &y, &w, &h);
        GUIRenderer_draw_frame(x, y, w, h);
    }

    GUIRenderer_send_buffer();
}

/* --- Input Handling --- */

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

            // Determine navigation destination based on component focus
            if (settings_page.focused_component ==
                (GUIComponent*)&settings_page.lbl_game_settings) {
                GameSettingsPage_enter();
            } else if (settings_page.focused_component ==
                       (GUIComponent*)&settings_page.lbl_device_settings) {
                DeviceSettingsPage_enter();
            }
            return;

        case BUTTON_CODE_CANCEL:
            MenuPage_enter();
            return;

        default:
            break;
    }

    if (next_focus != NULL) {
        AudioManager_play_sound(SOUND_UI_MOVE);
        settings_page.focused_component = next_focus;
        SettingsPage_draw();
    }
}

/* --- Page Lifecycle --- */

void SettingsPage_enter() {
    if (!is_initialized) {
        // 1. Component Initialization
        GUIVBox_init(&settings_page.root_vbox);
        GUILabel_init(&settings_page.lbl_game_settings, "Game Settings");
        GUILabel_init(&settings_page.lbl_device_settings, "Device Settings");

        // 2. Main Container Configuration
        GUI_SET_POS(&settings_page.root_vbox, 0, 0);
        GUI_SET_SIZE(&settings_page.root_vbox, 128, 64);
        GUI_SET_SPACING(&settings_page.root_vbox, 6);
        GUI_SET_PADDING(&settings_page.root_vbox, 4);

        // 3. Label Styling
        GUI_SET_FONT_SIZE(&settings_page.lbl_game_settings, 7);
        GUILabel_set_alignment(&settings_page.lbl_game_settings,
                               GUI_ALIGMNENT_CENTER);

        GUI_SET_FONT_SIZE(&settings_page.lbl_device_settings, 7);
        GUILabel_set_alignment(&settings_page.lbl_device_settings,
                               GUI_ALIGMNENT_CENTER);

        // 4. Hierarchy Construction
        GUI_ADD_CHILDREN(&settings_page.root_vbox,
                         &settings_page.lbl_game_settings,
                         &settings_page.lbl_device_settings);

        // 5. Layout Calculation
        GUI_UPDATE_LAYOUT(&settings_page.root_vbox);

        // 6. Navigation Logic (Vertical links with manual wrap-around)
        GUI_LINK_VERTICAL(&settings_page.lbl_game_settings,
                          &settings_page.lbl_device_settings);

        settings_page.lbl_device_settings.base.nav_down =
            (GUIComponent*)&settings_page.lbl_game_settings;
        settings_page.lbl_game_settings.base.nav_up =
            (GUIComponent*)&settings_page.lbl_device_settings;

        is_initialized = true;
    }

    // Default focus to the top option upon entry
    settings_page.focused_component =
        (GUIComponent*)&settings_page.lbl_game_settings;

    Page new_page = {.handle_input = SettingsPage_handle_input, .exit = NULL};

    PageManager_switch_page(&new_page);
    SettingsPage_draw();
}