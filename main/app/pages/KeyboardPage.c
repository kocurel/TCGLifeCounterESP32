#include "KeyboardPage.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "AudioManager.h"
#include "GUIRenderer.h"
#include "app/PageManager.h"

#define KB_ROWS 4
#define KB_COLS 7
#define MAX_NAME_LEN 11

// Definicja trybów klawiatury
typedef enum { KB_MODE_UPPER, KB_MODE_LOWER, KB_MODE_NUM } KeyboardMode;

static const char* kb_chars_upper[KB_ROWS][KB_COLS] = {
    {"A", "B", "C", "D", "E", "F", "G"},
    {"H", "I", "J", "K", "L", "M", "N"},
    {"O", "P", "Q", "R", "S", "T", "U"},
    {"V", "W", "X", "Y", "Z", "<", "OK"}};

static const char* kb_chars_num[KB_ROWS][KB_COLS] = {
    {"1", "2", "3", "4", "5", "6", "7"},
    {"8", "9", "0", "!", "@", "#", "$"},
    {"%", "&", "*", "(", ")", "-", "+"},
    {".", ",", "?", "/", "_", "<", "OK"}};

static char key_labels_cache[KB_ROWS][KB_COLS][4];

typedef struct {
    GUIVBox root;
    GUIVBox keyboard_grid;
    GUIHBox rows[KB_ROWS];
    GUILabel keys[KB_ROWS][KB_COLS];

    GUILabel title_lbl;
    GUILabel input_lbl;

    char buffer[MAX_NAME_LEN + 1];
    KeyboardCallback callback;
    GUILabel* selected_key;

    KeyboardMode mode;  // Główny stan trybu
} KeyboardContext;

static KeyboardContext ctx;
static bool is_initialized = false;

static void update_key_labels() {
    for (int r = 0; r < KB_ROWS; r++) {
        for (int c = 0; c < KB_COLS; c++) {
            // Zachowujemy funkcjonalność OK i Backspace
            if (c == 6 && r == 3) {  // OK
                snprintf(key_labels_cache[r][c], 4, "OK");
            } else if (c == 5 && r == 3) {  // <
                snprintf(key_labels_cache[r][c], 4, "<");
            } else {
                switch (ctx.mode) {
                    case KB_MODE_NUM:
                        snprintf(key_labels_cache[r][c], 4, "%s",
                                 kb_chars_num[r][c]);
                        break;
                    case KB_MODE_UPPER:
                        snprintf(key_labels_cache[r][c], 4, "%s",
                                 kb_chars_upper[r][c]);
                        break;
                    case KB_MODE_LOWER:
                        key_labels_cache[r][c][0] =
                            tolower((unsigned char)kb_chars_upper[r][c][0]);
                        key_labels_cache[r][c][1] = '\0';
                        break;
                }
            }
            GUI_SET_TEXT(&ctx.keys[r][c], key_labels_cache[r][c]);
        }
    }
}

static void KeyboardPage_draw() {
    GUIRenderer_clear_buffer();

    char display_buf[MAX_NAME_LEN + 2];
    snprintf(display_buf, sizeof(display_buf), "%s_", ctx.buffer);
    GUI_SET_TEXT(&ctx.input_lbl, display_buf);

    GUI_DRAW(&ctx.root);
    GUI_DRAW(&ctx.keyboard_grid);

    if (ctx.selected_key != NULL) {
        uint8_t x, y, w, h;
        GUIComponent_get_xywh((GUIComponent*)ctx.selected_key, &x, &y, &w, &h);
        GUIRenderer_draw_frame(x, y, w, h);
    }

    GUIRenderer_set_font_size(6);
    // Dynamiczny indykator trybu
    const char* mode_str = (ctx.mode == KB_MODE_UPPER)   ? "CAP"
                           : (ctx.mode == KB_MODE_LOWER) ? "low"
                                                         : "123";
    GUIRenderer_draw_str(110, 8, mode_str);

    GUIRenderer_send_buffer();
}

static void KeyboardPage_handle_input(ButtonCode button) {
    GUIComponent* next = NULL;

    switch (button) {
        case BUTTON_CODE_UP:
            next = ctx.selected_key->base.nav_up;
            break;
        case BUTTON_CODE_DOWN:
            next = ctx.selected_key->base.nav_down;
            break;
        case BUTTON_CODE_LEFT:
            next = ctx.selected_key->base.nav_left;
            break;
        case BUTTON_CODE_RIGHT:
            next = ctx.selected_key->base.nav_right;
            break;

        // Przełączanie trybów: CAP -> low -> 123
        case BUTTON_CODE_SET:
            ctx.mode = (ctx.mode + 1) % 3;
            update_key_labels();
            KeyboardPage_draw();
            break;

        // Backspace
        case BUTTON_CODE_MENU:
            int len = strlen(ctx.buffer);
            if (len > 0) {
                ctx.buffer[len - 1] = '\0';
                KeyboardPage_draw();
            }
            break;

        case BUTTON_CODE_ACCEPT:
            const char* val = ctx.selected_key->text;

            if (strcmp(val, "OK") == 0) {
                AudioManager_play_sound(SOUND_UI_SELECT);
                if (ctx.callback) ctx.callback(ctx.buffer);
                return;
            } else if (strcmp(val, "<") == 0) {
                int len = strlen(ctx.buffer);
                if (len > 0) ctx.buffer[len - 1] = '\0';
            } else {
                int len = strlen(ctx.buffer);
                if (len < MAX_NAME_LEN)
                    strcat(ctx.buffer, val);
                else
                    AudioManager_play_sound(SOUND_UI_ERROR);
            }
            KeyboardPage_draw();
            break;
        case BUTTON_CODE_CANCEL:
            AudioManager_play_sound(SOUND_UI_CANCEL);
            if (ctx.callback) ctx.callback(NULL);
            return;
        default:
            break;
    }

    if (next != NULL) {
        ctx.selected_key = (GUILabel*)next;
        KeyboardPage_draw();
    }
}

void KeyboardPage_enter(const char* title, const char* initial_text,
                        KeyboardCallback callback) {
    ctx.mode = KB_MODE_UPPER;

    if (!is_initialized) {
        GUIVBox_init(&ctx.root);
        GUI_SET_POS(&ctx.root, 0, 0);
        GUI_SET_SIZE(&ctx.root, 128, 20);
        GUI_SET_PADDING(&ctx.root, 1);
        GUI_SET_SPACING(&ctx.root, 0);

        GUILabel_init(&ctx.title_lbl, "");
        GUI_SET_FONT_SIZE(&ctx.title_lbl, 6);
        GUILabel_init(&ctx.input_lbl, "_");
        GUI_SET_FONT_SIZE(&ctx.input_lbl, 7);
        GUI_ADD_CHILDREN(&ctx.root, &ctx.title_lbl, &ctx.input_lbl);

        GUIVBox_init(&ctx.keyboard_grid);
        GUI_SET_POS(&ctx.keyboard_grid, 0, 20);
        GUI_SET_SIZE(&ctx.keyboard_grid, 128, 44);
        GUI_SET_PADDING(&ctx.keyboard_grid, 0);
        GUI_SET_SPACING(&ctx.keyboard_grid, 0);

        for (int r = 0; r < KB_ROWS; r++) {
            GUIHBox_init(&ctx.rows[r]);
            GUI_SET_SPACING(&ctx.rows[r], 1);
            GUI_ADD_CHILD(&ctx.keyboard_grid, &ctx.rows[r]);

            for (int c = 0; c < KB_COLS; c++) {
                GUILabel* key = &ctx.keys[r][c];
                GUILabel_init(key, key_labels_cache[r][c]);
                GUI_SET_SIZE(key, 17, 11);
                GUI_SET_FONT_SIZE(key, 6);
                GUILabel_set_alignment(key, GUI_ALIGMNENT_CENTER);
                GUI_ADD_CHILD(&ctx.rows[r], key);
            }
        }

        GUI_UPDATE_LAYOUT(&ctx.root);
        GUI_UPDATE_LAYOUT(&ctx.keyboard_grid);

        for (int r = 0; r < KB_ROWS; r++) {
            for (int c = 0; c < KB_COLS; c++) {
                GUILabel* curr = &ctx.keys[r][c];
                if (c > 0)
                    curr->base.nav_left = (GUIComponent*)&ctx.keys[r][c - 1];
                else
                    // Wrap around on the left
                    curr->base.nav_left =
                        (GUIComponent*)&ctx.keys[r][KB_COLS - 1];

                if (c < KB_COLS - 1)
                    curr->base.nav_right = (GUIComponent*)&ctx.keys[r][c + 1];
                else
                    // wrap around on the right
                    curr->base.nav_right = (GUIComponent*)&ctx.keys[r][0];

                if (r > 0)
                    curr->base.nav_up = (GUIComponent*)&ctx.keys[r - 1][c];
                else
                    // Wrap around on the top
                    curr->base.nav_up =
                        (GUIComponent*)&ctx.keys[KB_ROWS - 1][c];

                if (r < KB_ROWS - 1)
                    curr->base.nav_down = (GUIComponent*)&ctx.keys[r + 1][c];
                else
                    // Wrap around on the bottom
                    curr->base.nav_down = (GUIComponent*)&ctx.keys[0][c];
                is_initialized = true;
            }
        }
    }

    GUI_SET_TEXT(&ctx.title_lbl, title);
    ctx.callback = callback;
    if (initial_text) {
        strncpy(ctx.buffer, initial_text, MAX_NAME_LEN);
        ctx.buffer[MAX_NAME_LEN] = '\0';
    } else {
        ctx.buffer[0] = '\0';
    }

    update_key_labels();
    ctx.selected_key = &ctx.keys[0][0];

    Page page = {.handle_input = KeyboardPage_handle_input, .exit = NULL};
    PageManager_switch_page(&page);
    KeyboardPage_draw();
}