#include "GameDelegates.h"

#include <GUIFramework.h>
#include <stdio.h>
#include <string.h>

#include "Debug.h"
#include "model/Game.h"

/* --- External Dependencies --- */
extern Game game;

/* --- Data Access Delegates --- */

/**
 * Returns a pointer to a specific value in a player's data array
 */
void* delegate_get_player_value(void* data_source, int index) {
    struct Player* p = (Player*)data_source;
    if (index < 0 || index >= NUMBER_OF_VALUES) return NULL;
    return &p->values[index];
}

/**
 * Returns the constant number of values tracked per player
 */
int delegate_get_value_count(void* data) { return NUMBER_OF_VALUES; }

/**
 * Returns the current depth of the undo/redo change history
 */
int delegate_get_change_history_count(void* data) {
    return Game_get_change_history_count();
}

/**
 * Retrieves a historical value change by index
 */
void* delegate_get_value_change(void* context, int index) {
    return Game_get_change(index);
}

/* --- Internal Logic Helpers --- */

/**
 * Shortens a player name for list displays (Max 7 characters).
 * Format: "Short" or "Fi-st" for longer names.
 */
static void get_short_name(int player_id, char* out_buf) {
    const char* name = Game_get_player_name(player_id);
    if (!name) {
        strcpy(out_buf, "???");
        return;
    }

    size_t len = strlen(name);
    if (len <= 5) {
        strncpy(out_buf, name, 8);
    } else {
        snprintf(out_buf, 8, "%c%c-%c%c", name[0], name[1], name[len - 2],
                 name[len - 1]);
    }
}

/* --- Custom Rendering Delegates --- */

/**
 * Custom draw function for the History Page list items.
 * Formats entry as: "Receiver Source +Diff"
 */
void HistoryPage_draw_item(GUIList* list, int index, uint8_t x, uint8_t y,
                           uint8_t w, uint8_t h, bool is_selected) {
    int total_count = Game_get_change_history_count();

    // Specjalny opis dla początku gry
    if (index == total_count - 1) {
        GUIRenderer_set_font_size(6);
        // USUNIĘTO: if (is_selected) GUIRenderer_draw_str(x, y, ">");
        GUIRenderer_draw_str(x + 10, y, "--- Game Start ---");

        // Ramka wskazująca aktualny punkt w czasie (Undo pointer) zostaje,
        // ale zmieniamy jej szerokość, by nie kolidowała z animacją kursora.
        if (Game_get_current_undo_index() == index) {
            GUIRenderer_draw_frame(8, y - 9, 112, 11);
        }
        return;
    }

    ValueChange* change = Game_get_change(index);
    if (!change) return;

    char short_receiver[8];
    char short_source[24];
    get_short_name(change->player_index, short_receiver);

    if (change->value_index >= COMMANDER_DAMAGE_START_INDEX &&
        change->value_index < COMMANDER_DAMAGE_START_INDEX + 4) {
        int source_player_id =
            change->value_index - COMMANDER_DAMAGE_START_INDEX;
        char attacker_name[8];
        get_short_name(source_player_id, attacker_name);
        snprintf(short_source, sizeof(short_source), "from %s", attacker_name);
    } else {
        const char* v_name = Game_get_value_name(change->value_index);
        snprintf(short_source, sizeof(short_source), "%s",
                 v_name ? v_name : "???");
    }

    char line[64];
    snprintf(line, sizeof(line), "%s %s %+ld", short_receiver, short_source,
             (long)change->difference);

    GUIRenderer_set_font_size(6);
    // USUNIĘTO: if (is_selected) GUIRenderer_draw_str(x, y, ">");
    GUIRenderer_draw_str(x + 6, y, line);

    // Wskaźnik "teraz" w historii
    if (index == Game_get_current_undo_index()) {
        GUIRenderer_draw_frame(8, y - 9, 112, 11);
    }
}

/**
 * Formats a player's attribute value for the Player Page list
 */
char* delegate_format_player_value(void* data_source, int index) {
    if (index < 0 || index >= NUMBER_OF_VALUES) return NULL;

    // Pobieramy ID gracza ze źródła danych (w PlayerPage.c przekazujemy
    // (void*)player_id)
    int player_id = (int)(intptr_t)data_source;
    int32_t value =
        Game_get_value(player_id, index);  // Bezpieczne pobranie wartości

    static char buffer[32];

    if (index >= COMMANDER_DAMAGE_START_INDEX &&
        index < COMMANDER_DAMAGE_START_INDEX + 4) {
        int source_id = index - COMMANDER_DAMAGE_START_INDEX;
        char short_name[8];
        get_short_name(source_id, short_name);
        snprintf(buffer, sizeof(buffer), "from %s: %ld", short_name,
                 (long)value);
    } else {
        const char* value_name = Game_get_value_name(index);
        snprintf(buffer, sizeof(buffer), "%s: %ld",
                 value_name ? value_name : "???", (long)value);
    }

    return buffer;
}