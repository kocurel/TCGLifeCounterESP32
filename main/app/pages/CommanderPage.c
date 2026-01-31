#include "CommanderPage.h"

#include <stdio.h>

#include "Debug.h"
#include "GUIFramework.h"
#include "MainPage.h"
#include "ValueEditorPage.h"
#include "app/PageManager.h"
#include "model/Game.h"
#include "string.h"

typedef struct {
    GUILabel labels[4][4];
    GUIVBox root;
    GUIHBox row_top;
    GUIHBox row_bot;
    GUIHBox block_p1;
    GUIHBox block_p2;
    GUIHBox block_p3;
    GUIHBox block_p4;
    GUIVBox block_p10;
    GUIVBox block_p11;
    GUIVBox block_p20;
    GUIVBox block_p21;
    GUIVBox block_p30;
    GUIVBox block_p31;
    GUIVBox block_p40;
    GUIVBox block_p41;
    GUILabel* selected_label;

    // --- Transaction Buffering ---
    // [Player receiving Dmg][Source of Dmg]
    int32_t buffered_val[4][4];
    bool is_dirty[4][4];
} CommanderPageData;

static CommanderPageData commander_page = {0};
static bool is_initialized = false;
#define BUFFER_SIZE 24
static char buffer[BUFFER_SIZE];

// --- Internal Helpers ---

static void CommanderPage_get_ids(int* player_id, int* source_id) {
    ptrdiff_t offset =
        commander_page.selected_label - &commander_page.labels[0][0];
    *player_id = offset / 4;
    *source_id = offset % 4;
}

static int32_t CommanderPage_get_value_id(int source_id) {
    // Maps 0-3 to the actual indices in the Player's value array
    return COMMANDER_DAMAGE_START_INDEX + source_id;
}

// Funkcja zatwierdzająca zmiany (Commit)
// Oblicza deltę i aktualizuje zarówno Commander Damage jak i HP gracza
static void CommanderPage_commit_cell(int p_id, int s_id) {
    if (p_id == s_id) return;  // "ME" field guard
    if (!commander_page.is_dirty[p_id][s_id]) return;

    int32_t new_cmd_val = commander_page.buffered_val[p_id][s_id];

    // 1. Nakładamy twardy limit (Clamping)
    if (new_cmd_val > 999) new_cmd_val = 999;
    if (new_cmd_val < 0) new_cmd_val = 0;

    int32_t val_id = CommanderPage_get_value_id(s_id);
    int32_t old_cmd_val = Game_get_value(p_id, val_id);

    // 2. Obliczamy różnicę
    int32_t diff = new_cmd_val - old_cmd_val;

    if (diff != 0) {
        // 3. Aktualizujemy licznik Commander Damage w modelu
        Game_set_value(new_cmd_val, p_id, val_id);

        // 4. Inwersyjna korekta HP
        // Jeśli diff > 0 (obrażenia rosną), życie maleje.
        int32_t current_hp = Game_get_value(p_id, 0);
        Game_set_value(current_hp - diff, p_id, 0);
    }

    // 5. Czyścimy flagę
    commander_page.is_dirty[p_id][s_id] = false;
}

static void CommanderPage_handle_input(ButtonCode button);

static void CommanderPage_draw() {
    GUIRenderer_clear_buffer();

    char buf[BUFFER_SIZE];
    // 1. Rysowanie siatki (Grid)
    for (int p = 0; p < 4; p++) {
        for (int s = 0; s < 4; s++) {
            // Jeśli gracz bije samego siebie -> wyświetlamy ME
            if (p == s) {
                GUI_SET_TEXT(&commander_page.labels[p][s], "ME");
            } else {
                // Logika wyświetlania: Bufor vs Model
                // Tutaj wyświetlamy samą liczbę (bez gwiazdki), tak jak na
                // MainPage
                if (commander_page.is_dirty[p][s]) {
                    snprintf(buf, BUFFER_SIZE, "%ld",
                             (long)commander_page.buffered_val[p][s]);
                } else {
                    snprintf(buf, BUFFER_SIZE, "%ld",
                             (long)Game_get_commander_damage(p, s));
                }
                GUI_SET_TEXT(&commander_page.labels[p][s], buf);
            }
            GUI_DRAW(&commander_page.labels[p][s]);
        }
    }

    // 2. Rysowanie ramki zaznaczenia (Selection Frame)
    if (commander_page.selected_label != NULL) {
        uint8_t x, y, w, h;
        GUIComponent_get_xywh((GUIComponent*)commander_page.selected_label, &x,
                              &y, &w, &h);

        // Frame 1 pixel outside component bounds
        GUIRenderer_draw_frame(x, y - 1, w, h);
    }

    // 3. Rysowanie Delty (Floating Indicator)
    if (commander_page.selected_label != NULL) {
        // Musimy ustalić, na której komórce jesteśmy
        ptrdiff_t offset =
            commander_page.selected_label - &commander_page.labels[0][0];
        int p_id = offset / 4;
        int s_id = offset % 4;

        // Sprawdzamy, czy ta komórka ma niezapisane zmiany
        if (commander_page.is_dirty[p_id][s_id]) {
            int32_t current_model_val = Game_get_commander_damage(p_id, s_id);
            int32_t delta =
                commander_page.buffered_val[p_id][s_id] - current_model_val;

            if (delta != 0) {
                char delta_str[16];
                snprintf(delta_str, sizeof(delta_str), "%+ld", (long)delta);

                // Obliczenia wymiarów (identyczne jak w MainPage)
                uint8_t str_w = strlen(delta_str) * 6;  // Estymacja szerokości
                uint8_t box_w = str_w + 6;              // Margines poziomy
                uint8_t box_h = 12;                     // Wysokość ramki

                // Środek ekranu
                uint8_t x = 64 - (box_w / 2);
                uint8_t y = 32 - (box_h / 2) - 1;

                // A. Czyścimy tło (czarny prostokąt)
                GUIRenderer_set_color(0);
                GUIRenderer_draw_box(x, y, box_w, box_h);
                GUIRenderer_set_color(1);

                // B. Rysujemy ramkę
                GUIRenderer_draw_frame(x, y, box_w, box_h);

                GUIRenderer_set_font_size(7);
                // C. Rysujemy tekst
                // y + box_h - 3 to idealna pozycja dla fontu rysowanego od
                // baseline
                GUIRenderer_draw_str(x + 3, y + box_h - 3, delta_str);
            }
        }
    }

    GUIRenderer_send_buffer();
}

static void CommanderPage_editor_callback(int32_t new_value) {
    int p_id, s_id;
    CommanderPage_get_ids(&p_id, &s_id);

    // W przypadku edytora (SET), robimy ręczny commit logiki inwersji HP
    // Ale musimy to zrobić sprytnie, bo new_value to już NOWA wartość.

    // 1. Pobieramy STARĄ wartość z modelu (ignorujemy bufor, bo editor
    // nadpisuje wszystko)
    int32_t val_id = CommanderPage_get_value_id(s_id);
    int32_t old_cmd_val = Game_get_value(p_id, val_id);

    // 2. Obliczamy różnicę
    int32_t diff = new_value - old_cmd_val;

    // 3. Zapisujemy do modelu
    Game_set_value(new_value, p_id, val_id);

    // 4. Korekta HP
    if (diff != 0) {
        int32_t current_hp = Game_get_value(p_id, 0);
        Game_set_value(current_hp - diff, p_id, 0);
    }

    // 5. Czyścimy bufor, żeby nie było konfliktów po powrocie
    commander_page.is_dirty[p_id][s_id] = false;

    // Return to this page
    Page page = {.handle_input = CommanderPage_handle_input};
    PageManager_switch_page(&page);
    CommanderPage_draw();
}

static void CommanderPage_handle_input(ButtonCode button) {
    GUIComponent* next = NULL;

    // Pobieramy ID aktualnie wybranego pola
    int p_id, s_id;
    CommanderPage_get_ids(&p_id, &s_id);
    bool is_me_field = (p_id == s_id);

    switch (button) {
        case BUTTON_CODE_UP:
            next = commander_page.selected_label->base.nav_up;
            break;
        case BUTTON_CODE_DOWN:
            next = commander_page.selected_label->base.nav_down;
            break;
        case BUTTON_CODE_LEFT:
            next = commander_page.selected_label->base.nav_left;
            break;
        case BUTTON_CODE_RIGHT:
            next = commander_page.selected_label->base.nav_right;
            break;

        case BUTTON_CODE_CANCEL:
            // Commit przed wyjściem
            CommanderPage_commit_cell(p_id, s_id);
            MainPage_enter();
            return;

        case BUTTON_CODE_ACCEPT: {
            if (is_me_field) return;

            // --- QUICK EDIT: Buffer Logic ---
            // 1. Init bufora jeśli czysty
            if (!commander_page.is_dirty[p_id][s_id]) {
                commander_page.buffered_val[p_id][s_id] =
                    Game_get_commander_damage(p_id, s_id);
                commander_page.is_dirty[p_id][s_id] = true;
            }

            // 2. Modyfikacja bufora
            commander_page.buffered_val[p_id][s_id]++;

            // 3. Draw zaktualizuje gwiazdkę
            CommanderPage_draw();
            break;
        }

        case BUTTON_CODE_SET: {
            if (is_me_field) return;

            // Commit przed otwarciem edytora (żeby stan był spójny)
            CommanderPage_commit_cell(p_id, s_id);

            int32_t val_id = CommanderPage_get_value_id(s_id);
            ValueEditorPage_enter(Game_get_player_name(p_id), "Cmd Dmg", val_id,
                                  Game_get_value(p_id, val_id),
                                  CommanderPage_editor_callback);
            break;
        }
        default:
            break;
    }

    if (next) {
        // --- ZMIANA KONTEKSTU ---
        // Zapisujemy zmiany starej komórki
        CommanderPage_commit_cell(p_id, s_id);

        commander_page.selected_label = (GUILabel*)next;
        CommanderPage_draw();
    }
}

void CommanderPage_enter(int initial_player_id) {
    LOG_DEBUG("CommanderPage_enter", "initial_player_id: %d",
              initial_player_id);

    // Reset dirty flags przy wejściu
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            commander_page.is_dirty[i][j] = false;
            commander_page.buffered_val[i][j] = 0;
        }
    }

    if (!is_initialized) {
        GUIVBox_init(&commander_page.root);
        GUIHBox_init(&commander_page.row_top);
        GUIHBox_init(&commander_page.row_bot);
        GUIHBox_init(&commander_page.block_p1);
        GUIHBox_init(&commander_page.block_p2);
        GUIHBox_init(&commander_page.block_p3);
        GUIHBox_init(&commander_page.block_p4);
        GUIVBox_init(&commander_page.block_p10);
        GUIVBox_init(&commander_page.block_p11);
        GUIVBox_init(&commander_page.block_p20);
        GUIVBox_init(&commander_page.block_p21);
        GUIVBox_init(&commander_page.block_p30);
        GUIVBox_init(&commander_page.block_p31);
        GUIVBox_init(&commander_page.block_p40);
        GUIVBox_init(&commander_page.block_p41);

        for (int player_id = 0; player_id < 4; player_id++) {
            for (int source_id = 0; source_id < 4; source_id++) {
                GUILabel_init(&commander_page.labels[player_id][source_id], "");
                GUI_SET_FONT_SIZE(&commander_page.labels[player_id][source_id],
                                  8);
                GUILabel_set_alignment(
                    &commander_page.labels[player_id][source_id],
                    GUI_ALIGMNENT_CENTER);

                snprintf(buffer, BUFFER_SIZE, "%ld",
                         Game_get_commander_damage(player_id, source_id));
                GUI_SET_TEXT(&commander_page.labels[player_id][source_id],
                             buffer);
            }
        }
        // vertical root (4 columns)
        GUI_ADD_CHILDREN(&commander_page.root, &commander_page.row_top,
                         &commander_page.row_bot);
        // 2 top horizontal rows
        GUI_ADD_CHILDREN(&commander_page.row_top, &commander_page.block_p1,
                         &commander_page.block_p2);
        // 2 bottom horizontal rows
        GUI_ADD_CHILDREN(&commander_page.row_bot, &commander_page.block_p3,
                         &commander_page.block_p4);
        // player 1 two vertical columns
        GUI_ADD_CHILDREN(&commander_page.block_p1, &commander_page.block_p10,
                         &commander_page.block_p11);
        GUI_ADD_CHILDREN(&commander_page.block_p2, &commander_page.block_p20,
                         &commander_page.block_p21);
        // player 3 two vertical columns
        GUI_ADD_CHILDREN(&commander_page.block_p3, &commander_page.block_p30,
                         &commander_page.block_p31);
        // player 4 two vertical columns
        GUI_ADD_CHILDREN(&commander_page.block_p4, &commander_page.block_p40,
                         &commander_page.block_p41);
        // player 1 labels s1 s3
        GUI_ADD_CHILDREN(&commander_page.block_p10,
                         &commander_page.labels[0][0],
                         &commander_page.labels[0][2]);
        // player 1 labels s2 s4
        GUI_ADD_CHILDREN(&commander_page.block_p11,
                         &commander_page.labels[0][1],
                         &commander_page.labels[0][3]);
        // player 2 labels s1 s3
        GUI_ADD_CHILDREN(&commander_page.block_p20,
                         &commander_page.labels[1][0],
                         &commander_page.labels[1][2]);
        // player 2 labels s2 s4
        GUI_ADD_CHILDREN(&commander_page.block_p21,
                         &commander_page.labels[1][1],
                         &commander_page.labels[1][3]);
        // player 3 labels s1 s3
        GUI_ADD_CHILDREN(&commander_page.block_p30,
                         &commander_page.labels[2][0],
                         &commander_page.labels[2][2]);
        // player 3 labels s2 s4
        GUI_ADD_CHILDREN(&commander_page.block_p31,
                         &commander_page.labels[2][1],
                         &commander_page.labels[2][3]);
        // player 4 labels s1 s3
        GUI_ADD_CHILDREN(&commander_page.block_p40,
                         &commander_page.labels[3][0],
                         &commander_page.labels[3][2]);
        // player 4 labels s2 s4
        GUI_ADD_CHILDREN(&commander_page.block_p41,
                         &commander_page.labels[3][1],
                         &commander_page.labels[3][3]);

        GUI_SET_SIZE(&commander_page.root, 128, 64);
        GUI_SET_PADDING(&commander_page.root, 2);
        GUI_SET_SPACING(&commander_page.root, 12);
        GUI_SET_SPACING(&commander_page.row_top, 16);
        GUI_SET_SPACING(&commander_page.row_bot, 16);

        GUI_UPDATE_LAYOUT(&commander_page.root);

        // link player 1
        GUI_LINK_VERTICAL(&commander_page.labels[0][0],
                          &commander_page.labels[0][2]);
        GUI_LINK_VERTICAL(&commander_page.labels[0][1],
                          &commander_page.labels[0][3]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[0][0],
                            &commander_page.labels[0][1]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[0][2],
                            &commander_page.labels[0][3]);

        // link player 2
        GUI_LINK_VERTICAL(&commander_page.labels[1][0],
                          &commander_page.labels[1][2]);
        GUI_LINK_VERTICAL(&commander_page.labels[1][1],
                          &commander_page.labels[1][3]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[1][0],
                            &commander_page.labels[1][1]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[1][2],
                            &commander_page.labels[1][3]);

        // link player 3
        GUI_LINK_VERTICAL(&commander_page.labels[2][0],
                          &commander_page.labels[2][2]);
        GUI_LINK_VERTICAL(&commander_page.labels[2][1],
                          &commander_page.labels[2][3]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[2][0],
                            &commander_page.labels[2][1]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[2][2],
                            &commander_page.labels[2][3]);

        // link player 4
        GUI_LINK_VERTICAL(&commander_page.labels[3][0],
                          &commander_page.labels[3][2]);
        GUI_LINK_VERTICAL(&commander_page.labels[3][1],
                          &commander_page.labels[3][3]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[3][0],
                            &commander_page.labels[3][1]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[3][2],
                            &commander_page.labels[3][3]);

        // link player 1 with player 3
        GUI_LINK_VERTICAL(&commander_page.labels[0][2],
                          &commander_page.labels[2][0]);
        GUI_LINK_VERTICAL(&commander_page.labels[0][3],
                          &commander_page.labels[2][1]);
        // link player 1 with player 2
        GUI_LINK_HORIZONTAL(&commander_page.labels[0][1],
                            &commander_page.labels[1][0]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[0][3],
                            &commander_page.labels[1][2]);
        // link player 2 with player 4
        GUI_LINK_VERTICAL(&commander_page.labels[1][2],
                          &commander_page.labels[3][0]);
        GUI_LINK_VERTICAL(&commander_page.labels[1][3],
                          &commander_page.labels[3][1]);
        // link player 3 with player 4
        GUI_LINK_HORIZONTAL(&commander_page.labels[2][1],
                            &commander_page.labels[3][0]);
        GUI_LINK_HORIZONTAL(&commander_page.labels[2][3],
                            &commander_page.labels[3][2]);

        is_initialized = true;
    }
    // SANITY CHECK: Ensure ID is 0-3
    if (initial_player_id < 0 || initial_player_id > 3) {
        initial_player_id = 0;
    }

    commander_page.selected_label =
        &commander_page.labels[initial_player_id][0];

    Page new_page = {0};
    new_page.handle_input = CommanderPage_handle_input;
    PageManager_switch_page(&new_page);
    CommanderPage_draw();
}