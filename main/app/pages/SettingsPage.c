#include "SettingsPage.h"

#include <math.h>
#include <stdio.h>

#include "AudioManager.h"
#include "DeviceSettingsPage.h"
#include "GUIFramework.h"
#include "GameSettingsPage.h"
#include "MenuPage.h"
#include "app/PageManager.h"

typedef struct {
    GUIComponent* focused_component;
    GUIVBox root_vbox;
    GUILabel lbl_game_settings;
    GUILabel lbl_device_settings;

    float anim_x, anim_y, anim_w, anim_h;
    bool needs_redraw;
} SettingsPageData;

static SettingsPageData settings_page = {0};
static bool is_initialized = false;

static void SettingsPage_draw() {
    GUIRenderer_clear_buffer();

    GUI_DRAW(&settings_page.root_vbox);

    /* Render selection frame with float rounding for smooth sub-pixel movement
     */
    GUIRenderer_draw_frame((uint8_t)(settings_page.anim_x + 0.5f),
                           (uint8_t)(settings_page.anim_y + 0.5f),
                           (uint8_t)(settings_page.anim_w + 0.5f),
                           (uint8_t)(settings_page.anim_h + 0.5f));

    GUIRenderer_send_buffer();
}

static void SettingsPage_on_tick(uint32_t delta_ms) {
    if (settings_page.focused_component != NULL) {
        uint8_t tx, ty, tw, th;
        GUIComponent_get_xywh(settings_page.focused_component, &tx, &ty, &tw,
                              &th);

        const float base_speed = 0.25f;
        const float factor = (float)delta_ms / 20.0f;
        float final_speed = base_speed * factor;
        if (final_speed > 0.9f) final_speed = 0.9f;

        const float old_x = settings_page.anim_x;
        const float old_y = settings_page.anim_y;

        /* Interpolate selection frame position and dimensions */
        settings_page.anim_x +=
            ((float)tx - settings_page.anim_x) * final_speed;
        settings_page.anim_y +=
            ((float)ty - settings_page.anim_y) * final_speed;
        settings_page.anim_w +=
            ((float)tw - settings_page.anim_w) * final_speed;
        settings_page.anim_h +=
            ((float)th - settings_page.anim_h) * final_speed;

        /* Trigger redraw if animation is active */
        if (fabsf(settings_page.anim_x - old_x) > 0.05f ||
            fabsf(settings_page.anim_y - old_y) > 0.05f) {
            settings_page.needs_redraw = true;
        }
    }

    if (settings_page.needs_redraw) {
        SettingsPage_draw();
        settings_page.needs_redraw = false;
    }
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
        settings_page.focused_component = next_focus;
        /* Animation target updated; on_tick handles subsequent redraws */
    }
}

void SettingsPage_enter() {
    if (!is_initialized) {
        GUIVBox_init(&settings_page.root_vbox);
        GUILabel_init(&settings_page.lbl_game_settings, "Game Settings");
        GUILabel_init(&settings_page.lbl_device_settings, "Device Settings");

        GUI_SET_POS(&settings_page.root_vbox, 0, 0);
        GUI_SET_SIZE(&settings_page.root_vbox, 128, 64);
        GUI_SET_SPACING(&settings_page.root_vbox, 6);
        GUI_SET_PADDING(&settings_page.root_vbox, 10);

        GUI_SET_FONT_SIZE(&settings_page.lbl_game_settings, 7);
        GUILabel_set_alignment(&settings_page.lbl_game_settings,
                               GUI_ALIGMNENT_CENTER);
        GUI_SET_FONT_SIZE(&settings_page.lbl_device_settings, 7);
        GUILabel_set_alignment(&settings_page.lbl_device_settings,
                               GUI_ALIGMNENT_CENTER);

        GUI_ADD_CHILDREN(&settings_page.root_vbox,
                         &settings_page.lbl_game_settings,
                         &settings_page.lbl_device_settings);

        GUI_UPDATE_LAYOUT(&settings_page.root_vbox);

        GUI_LINK_VERTICAL(&settings_page.lbl_game_settings,
                          &settings_page.lbl_device_settings);

        /* Wrap around navigation */
        settings_page.lbl_device_settings.base.nav_down =
            (GUIComponent*)&settings_page.lbl_game_settings;
        settings_page.lbl_game_settings.base.nav_up =
            (GUIComponent*)&settings_page.lbl_device_settings;

        is_initialized = true;
    }

    settings_page.focused_component =
        (GUIComponent*)&settings_page.lbl_game_settings;

    /* Initialize animation state with immediate snap to target */
    uint8_t x, y, w, h;
    GUIComponent_get_xywh(settings_page.focused_component, &x, &y, &w, &h);
    settings_page.anim_x = (float)x;
    settings_page.anim_y = (float)y;
    settings_page.anim_w = (float)w;
    settings_page.anim_h = (float)h;
    settings_page.needs_redraw = false;

    Page new_page = {.handle_input = SettingsPage_handle_input,
                     .on_tick = SettingsPage_on_tick};

    PageManager_switch_page(&new_page);
    SettingsPage_draw();
}