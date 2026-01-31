#include "MainPage.h"

#include <stdio.h>

#include "ChangeHistoryPage.h"
#include "CommanderPage.h"
#include "GUIFramework.h"
#include "MenuPage.h"
#include "PlayerPage.h"
#include "ValueEditorPage.h"
#include "app/PageManager.h"
#include "model/Game.h"
#include "model/Settings.h"
#include "string.h"

typedef struct {
    GUIComponent* focused_component;
    GUIVBox root_vbox;
    GUIHBox row_top;
    GUIHBox row_bot;

    GUIVBox box_p1;
    GUIVBox box_p2;
    GUIVBox box_p3;
    GUIVBox box_p4;

    GUILabel lbl_name_p1;
    GUILabel lbl_name_p2;
    GUILabel lbl_name_p3;
    GUILabel lbl_name_p4;

    GUILabel lbl_hp_p1;
    GUILabel lbl_hp_p2;
    GUILabel lbl_hp_p3;
    GUILabel lbl_hp_p4;

    // --- Transaction Buffering ---
    int32_t buffered_hp[4];  // Tymczasowa wartość HP podczas szybkiej edycji
    bool is_dirty[4];        // Flaga: czy wartość różni się od tej w modelu?
    uint32_t idle_timer_ms;
} MainPageData;

static MainPageData main_page = {0};
static bool is_initialized = false;
static int current_layout_mode = 0;

// --- Internal Helpers ---

static int MainPage_get_focused_player_id() {
    if (main_page.focused_component == (GUIComponent*)&main_page.box_p1)
        return 0;
    if (main_page.focused_component == (GUIComponent*)&main_page.box_p2)
        return 1;
    if (main_page.focused_component == (GUIComponent*)&main_page.box_p3)
        return 2;
    if (main_page.focused_component == (GUIComponent*)&main_page.box_p4)
        return 3;
    return 0;
}

// Funkcja zatwierdzająca zmiany do modelu (tworzy wpis w historii)
static void MainPage_commit_changes(int player_id) {
    if (player_id < 0 || player_id > 3) return;

    // Jeśli nie ma zmian ("brudnej" flagi), nic nie rób
    if (!main_page.is_dirty[player_id]) return;

    int32_t current_model_val = Game_get_value(player_id, 0);
    int32_t new_val = main_page.buffered_hp[player_id];

    // Zapisz tylko jeśli wartość faktycznie jest inna
    if (current_model_val != new_val) {
        Game_set_value(new_val, player_id, 0);
    }

    // Wyczyść flagę - jesteśmy zsynchronizowani
    main_page.is_dirty[player_id] = false;
}

static bool MainPage_is_player_dead(int player_id) {
    GameSettings settings = SettingsModel_get();
    // Tutaj sprawdzamy wartość z modelu (bo śmierć jest "oficjalna")
    // Ale można by dyskutować, czy buffered_hp <= 0 też powinno pokazywać KO.
    // Na razie trzymamy się modelu gry.
    if (settings.dead_at_zero && Game_get_value(player_id, 0) <= 0) return true;

    if (settings.cmd_dmg_rule) {
        for (int source = 0; source < 4; source++) {
            if (source != player_id &&
                Game_get_commander_damage(player_id, source) >= 21)
                return true;
        }
    }
    return false;
}

static void MainPage_update();
static void MainPage_handle_input(ButtonCode button);

static void MainPage_draw_monarch_indicator(int player_id, uint8_t x, uint8_t y,
                                            bool is_rotated) {
    if (!Game_is_monarch(player_id)) return;

    if (is_rotated) {
        GUIRenderer_draw_line(x, y, x + 4, y);  // Podstawa
        GUIRenderer_draw_pixel(x, y + 1);       // Czubek 1
        GUIRenderer_draw_pixel(x + 2, y + 1);   // Czubek 2
        GUIRenderer_draw_pixel(x + 4, y + 1);   // Czubek 3
    } else {
        // NORMALNIE: Podstawa na dole (y), czubki na górze (y-1).
        GUIRenderer_draw_line(x, y, x + 4, y);  // Podstawa
        GUIRenderer_draw_pixel(x, y - 1);       // Czubek 1
        GUIRenderer_draw_pixel(x + 2, y - 1);   // Czubek 2
        GUIRenderer_draw_pixel(x + 4, y - 1);   // Czubek 3
    }
}

static void MainPage_editor_callback(int32_t new_value) {
    int player_id = MainPage_get_focused_player_id();
    // Edytor wartości zawsze robi "twardy" zapis (commit)
    Game_set_value(new_value, player_id, 0);

    // Czyścimy bufor, żeby nie nadpisał edytora przy powrocie
    main_page.is_dirty[player_id] = false;

    Page new_page = {.handle_input = MainPage_handle_input, .exit = NULL};
    PageManager_switch_page(&new_page);
    MainPage_update();
}

static void MainPage_rebuild_layout(int player_count) {
    GUIVBox_init(&main_page.root_vbox);
    GUIHBox_init(&main_page.row_top);
    GUIHBox_init(&main_page.row_bot);

    GUIVBox_init(&main_page.box_p1);
    GUIVBox_init(&main_page.box_p2);
    GUIVBox_init(&main_page.box_p3);
    GUIVBox_init(&main_page.box_p4);

    GUI_SET_POS(&main_page.root_vbox, 0, 0);
    GUI_SET_SIZE(&main_page.root_vbox, 128, 64);
    GUI_SET_PADDING(&main_page.root_vbox, 0);

    GUIVBox* boxes[] = {&main_page.box_p1, &main_page.box_p2, &main_page.box_p3,
                        &main_page.box_p4};
    GUILabel* names[] = {&main_page.lbl_name_p1, &main_page.lbl_name_p2,
                         &main_page.lbl_name_p3, &main_page.lbl_name_p4};
    GUILabel* hps[] = {&main_page.lbl_hp_p1, &main_page.lbl_hp_p2,
                       &main_page.lbl_hp_p3, &main_page.lbl_hp_p4};

    for (int i = 0; i < 4; i++) {
        GUI_SET_SPACING(boxes[i], 0);
        GUI_SET_PADDING(boxes[i], 4);
        GUILabel_set_alignment(names[i], GUI_ALIGMNENT_CENTER);
        GUILabel_set_alignment(hps[i], GUI_ALIGMNENT_CENTER);
    }

    if (player_count == 2) {
        GUILabel_upside_down_en(&main_page.lbl_name_p1, true);
        GUILabel_upside_down_en(&main_page.lbl_hp_p1, true);
        GUI_ADD_CHILD(&main_page.box_p1, &main_page.lbl_hp_p1);
        GUI_ADD_CHILD(&main_page.box_p1, &main_page.lbl_name_p1);

        GUILabel_upside_down_en(&main_page.lbl_name_p2, false);
        GUILabel_upside_down_en(&main_page.lbl_hp_p2, false);
        GUI_ADD_CHILD(&main_page.box_p2, &main_page.lbl_name_p2);
        GUI_ADD_CHILD(&main_page.box_p2, &main_page.lbl_hp_p2);

        GUI_ADD_CHILD(&main_page.root_vbox, &main_page.box_p1);
        GUI_ADD_CHILD(&main_page.root_vbox, &main_page.box_p2);
        GUI_SET_SPACING(&main_page.root_vbox, 2);
        GUI_LINK_VERTICAL(&main_page.box_p1, &main_page.box_p2);

    } else {
        for (int i = 0; i < 2; i++) {
            GUILabel_upside_down_en(names[i], true);
            GUILabel_upside_down_en(hps[i], true);
            GUI_ADD_CHILD(boxes[i], hps[i]);
            GUI_ADD_CHILD(boxes[i], names[i]);
        }
        for (int i = 2; i < 4; i++) {
            GUILabel_upside_down_en(names[i], false);
            GUILabel_upside_down_en(hps[i], false);
            GUI_ADD_CHILD(boxes[i], names[i]);
            GUI_ADD_CHILD(boxes[i], hps[i]);
        }

        GUI_ADD_CHILDREN(&main_page.row_top, &main_page.box_p1,
                         &main_page.box_p2);
        GUI_ADD_CHILDREN(&main_page.row_bot, &main_page.box_p3,
                         &main_page.box_p4);
        GUI_ADD_CHILDREN(&main_page.root_vbox, &main_page.row_top,
                         &main_page.row_bot);

        GUI_SET_SPACING(&main_page.root_vbox, 2);
        GUI_SET_SPACING(&main_page.row_top, 1);
        GUI_SET_SPACING(&main_page.row_bot, 1);

        GUI_LINK_HORIZONTAL(&main_page.box_p1, &main_page.box_p2);
        GUI_LINK_HORIZONTAL(&main_page.box_p3, &main_page.box_p4);
        GUI_LINK_VERTICAL(&main_page.box_p1, &main_page.box_p3);
        GUI_LINK_VERTICAL(&main_page.box_p2, &main_page.box_p4);
    }

    GUI_UPDATE_LAYOUT(&main_page.root_vbox);
    current_layout_mode = player_count;
    main_page.focused_component = (GUIComponent*)&main_page.box_p1;
}

static void MainPage_update() {
    char str[32];
    int count = SettingsModel_get().player_count;

    GUILabel* labels_hp[4] = {&main_page.lbl_hp_p1, &main_page.lbl_hp_p2,
                              &main_page.lbl_hp_p3, &main_page.lbl_hp_p4};
    GUILabel* labels_name[4] = {&main_page.lbl_name_p1, &main_page.lbl_name_p2,
                                &main_page.lbl_name_p3, &main_page.lbl_name_p4};
    GUIVBox* boxes[4] = {&main_page.box_p1, &main_page.box_p2,
                         &main_page.box_p3, &main_page.box_p4};

    // 1. Rysowanie standardowe (Bez gwiazdek)
    for (int i = 0; i < count; i++) {
        if (MainPage_is_player_dead(i)) {
            GUI_SET_TEXT(labels_hp[i], "KO");
        } else {
            // Czy mamy niezapisane zmiany?
            if (main_page.is_dirty[i]) {
                // [ZMIANA] Wyświetlamy samą wartość z bufora, bez gwiazdki
                snprintf(str, sizeof(str), "%ld",
                         (long)main_page.buffered_hp[i]);
            } else {
                // Wartość z modelu
                snprintf(str, sizeof(str), "%ld", (long)Game_get_value(i, 0));
            }
            GUI_SET_TEXT(labels_hp[i], str);
        }
        GUI_SET_TEXT(labels_name[i], Game_get_player_name(i));
    }

    GUIRenderer_clear_buffer();
    GUI_DRAW(&main_page.root_vbox);

    // 2. Rysowanie korony (bez zmian)
    for (int i = 0; i < count; i++) {
        if (Game_is_monarch(i)) {
            uint8_t x, y, w, h;
            GUIComponent_get_xywh((GUIComponent*)boxes[i], &x, &y, &w, &h);
            bool rotated = (i < 2 && count == 4) || (i == 0 && count == 2);
            uint8_t crown_x = x + (w / 2) - 2;

            if (rotated) {
                MainPage_draw_monarch_indicator(i, crown_x, y + h - 5, true);
            } else {
                MainPage_draw_monarch_indicator(i, crown_x, y + 4, false);
            }
        }
    }

    // 3. Rysowanie ramki zaznaczenia
    if (main_page.focused_component != NULL) {
        uint8_t x, y, w, h;
        GUIComponent_get_xywh(main_page.focused_component, &x, &y, &w, &h);
        GUIRenderer_draw_frame(x, y, w, h);
    }

    // 4. [POPRAWKA] Rysowanie Delty na środku ekranu
    int pid = MainPage_get_focused_player_id();
    if (main_page.is_dirty[pid]) {
        int32_t current_model_val = Game_get_value(pid, 0);
        int32_t delta = main_page.buffered_hp[pid] - current_model_val;

        if (delta != 0) {
            char delta_str[16];
            snprintf(delta_str, sizeof(delta_str), "%+ld", (long)delta);

            // Obliczenia wymiarów
            uint8_t str_w = strlen(delta_str) * 6;  // Estymacja szerokości
            uint8_t box_w = str_w + 6;              // Margines poziomy
            uint8_t box_h = 11;                     // Wysokość ramki

            // Środek ekranu
            uint8_t x = 64 - (box_w / 2);
            uint8_t y = 32 - (box_h / 2);

            // 1. Czyścimy tło (czarny prostokąt)
            GUIRenderer_set_color(0);
            GUIRenderer_draw_box(x, y, box_w, box_h);
            GUIRenderer_set_color(1);
            GUIRenderer_set_font_size(7);
            GUIRenderer_draw_str(x + 3, y + box_h - 3, delta_str);
            GUIRenderer_draw_frame(x, y - 1, box_w, box_h);
        }
    }

    GUIRenderer_send_buffer();
}

static void MainPage_handle_input(ButtonCode button) {
    GUIComponent* next_focus = NULL;
    GameSettings settings = SettingsModel_get();
    int pid = MainPage_get_focused_player_id();

    switch (button) {
        case BUTTON_CODE_UP:
            next_focus = main_page.focused_component->nav_up;
            break;
        case BUTTON_CODE_DOWN:
            next_focus = main_page.focused_component->nav_down;
            break;
        case BUTTON_CODE_LEFT:
            next_focus = main_page.focused_component->nav_left;
            break;
        case BUTTON_CODE_RIGHT:
            next_focus = main_page.focused_component->nav_right;
            break;

        case BUTTON_CODE_ACCEPT:
            main_page.idle_timer_ms = 0;
            if (settings.quick_dmg_en) {
                // --- QUICK DMG: Buffer Logic ---
                int32_t step = 1;
                if (settings.starting_life > 1000)
                    step = 100;
                else if (settings.starting_life > 100)
                    step = 10;

                // 1. Jeśli to pierwsza zmiana, skopiuj z modelu do bufora
                if (!main_page.is_dirty[pid]) {
                    main_page.buffered_hp[pid] = Game_get_value(pid, 0);
                    main_page.is_dirty[pid] = true;
                }

                // 2. Modyfikuj TYLKO bufor
                main_page.buffered_hp[pid] -= step;

                // 3. Odśwież UI (pokaże gwiazdkę)
                MainPage_update();
            } else {
                // Przed wejściem głębiej, zatwierdź ewentualne wiszące zmiany
                MainPage_commit_changes(pid);
                PlayerPage_enter(pid);
            }
            break;

        case BUTTON_CODE_SET:
            // Zawsze commit przed zmianą kontekstu
            MainPage_commit_changes(pid);

            if (settings.quick_dmg_en) {
                PlayerPage_enter(pid);
            } else {
                ValueEditorPage_enter(
                    Game_get_player_name(pid), Game_get_value_name(0), 0,
                    Game_get_value(pid, 0), MainPage_editor_callback);
            }
            return;

        case BUTTON_CODE_CANCEL:
            // Commit przed wyjściem
            MainPage_commit_changes(pid);

            if (settings.cmd_mode_en) {
                CommanderPage_enter(pid);
            } else {
                // Przekazujemy MainPage_enter jako funkcję powrotną
                ChangeHistoryPage_enter(MainPage_enter);
            }
            return;

        case BUTTON_CODE_MENU:
            // Commit przed wyjściem
            MainPage_commit_changes(pid);
            MenuPage_enter();
            break;

        default:
            break;
    }

    if (next_focus != NULL) {
        // --- ZMIANA KONTEKSTU (Context Switch) ---
        // Użytkownik przesuwa fokus na innego gracza.
        // To moment, w którym "transakcja" starego gracza się kończy.
        MainPage_commit_changes(pid);

        main_page.focused_component = next_focus;
        MainPage_update();
    }
}

void MainPage_on_tick(uint32_t delta_ms) {
    bool changes_pending = false;

    // Sprawdzamy, czy którykolwiek gracz ma "brudne" dane
    for (int i = 0; i < 4; i++) {
        if (main_page.is_dirty[i]) {
            changes_pending = true;
            break;
        }
    }

    if (changes_pending) {
        main_page.idle_timer_ms += delta_ms;

        // Jeśli minęły 3 sekundy (3000 ms)
        if (main_page.idle_timer_ms >= 2000) {
            // Zapisujemy zmiany dla wszystkich graczy
            for (int i = 0; i < 4; i++) {
                MainPage_commit_changes(i);
            }

            main_page.idle_timer_ms = 0;

            // Odświeżamy ekran, aby zniknął "Floating Indicator" z deltą
            MainPage_update();
        }
    } else {
        // Resetujemy timer, jeśli nie ma nic do roboty
        main_page.idle_timer_ms = 0;
    }
}

void MainPage_enter() {
    GameSettings settings = SettingsModel_get();

    // Resetujemy flagi zmian przy wejściu na stronę,
    // żeby zaczynać z czystym stanem zgodnym z modelem.
    for (int i = 0; i < 4; i++) {
        main_page.is_dirty[i] = false;
        main_page.buffered_hp[i] = 0;
    }

    if (!is_initialized) {
        GUILabel_init(&main_page.lbl_name_p1, "");
        GUI_SET_FONT_SIZE(&main_page.lbl_name_p1, 6);
        GUILabel_init(&main_page.lbl_name_p2, "");
        GUI_SET_FONT_SIZE(&main_page.lbl_name_p2, 6);
        GUILabel_init(&main_page.lbl_name_p3, "");
        GUI_SET_FONT_SIZE(&main_page.lbl_name_p3, 6);
        GUILabel_init(&main_page.lbl_name_p4, "");
        GUI_SET_FONT_SIZE(&main_page.lbl_name_p4, 6);

        GUILabel_init(&main_page.lbl_hp_p1, "");
        GUI_SET_FONT_SIZE(&main_page.lbl_hp_p1, 10);
        GUILabel_init(&main_page.lbl_hp_p2, "");
        GUI_SET_FONT_SIZE(&main_page.lbl_hp_p2, 10);
        GUILabel_init(&main_page.lbl_hp_p3, "");
        GUI_SET_FONT_SIZE(&main_page.lbl_hp_p3, 10);
        GUILabel_init(&main_page.lbl_hp_p4, "");
        GUI_SET_FONT_SIZE(&main_page.lbl_hp_p4, 10);

        // Bateria usunięta z inicjalizacji widocznej, ale struktura została
        // w DataStruct (można zostawić, nieużywana nie szkodzi).
        is_initialized = true;
    }

    if (current_layout_mode != settings.player_count) {
        MainPage_rebuild_layout(settings.player_count);
    }

    Page new_page = {.handle_input = MainPage_handle_input,
                     .on_tick = MainPage_on_tick,
                     .exit = NULL};
    PageManager_switch_page(&new_page);
    MainPage_update();
}