#include "GameDelegates.h"

#include <GUIFramework.h>
#include <stdio.h>
#include <string.h>

#include "Debug.h"
#include "model/Game.h"

extern Game game;

void* delegate_get_player_value(void* data_source, int index) {
    struct Player* p = (Player*)data_source;
    if (index < 0 || index >= NUMBER_OF_VALUES) return NULL;
    return &p->values[index];
}

int delegate_get_value_count(void* data) { return NUMBER_OF_VALUES; }

int delegate_get_change_history_count(void* data) {
    return Game_get_change_history_count();
}

void* delegate_get_value_change(void* context, int index) {
    return Game_get_change(index);
}

// Funkcja pomocnicza, aby utrzymać kod w czystości (DRY - Don't Repeat
// Yourself)
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

void HistoryPage_draw_item(GUIList* list, int index, uint8_t x, uint8_t y,
                           uint8_t w, uint8_t h, bool is_selected) {
    int total_count = Game_get_change_history_count();

    if (index == total_count - 1) {
        GUIRenderer_set_font_size(6);
        if (is_selected) GUIRenderer_draw_str(x, y, ">");
        GUIRenderer_draw_str(x + 10, y, "--- Game Start ---");
        if (Game_get_current_undo_index() == total_count - 1) {
            LOG_DEBUG("HistoryPage_draw_item", "drawing frame");
            GUIRenderer_draw_frame(0, y - 9, 128, 12);
            return;
        }
    }

    ValueChange* change = Game_get_change(index);
    if (!change) return;

    char short_receiver[8];
    char short_source[24];  // Nieco większy bufor na wypadek dopisku "Cmd"

    // 1. Skrócona nazwa gracza, który otrzymał zmianę
    get_short_name(change->player_index, short_receiver);

    // 2. Logika rozpoznawania źródła (Commander Damage vs Zwykłe wartości)
    if (change->value_index >= COMMANDER_DAMAGE_START_INDEX &&
        change->value_index < COMMANDER_DAMAGE_START_INDEX + 4) {
        // Obliczamy ID gracza, który zadał obrażenia
        int source_player_id =
            change->value_index - COMMANDER_DAMAGE_START_INDEX;
        char attacker_name[8];
        get_short_name(source_player_id, attacker_name);

        // Formatujemy jako "od Kogoś"
        snprintf(short_source, sizeof(short_source), "from %s", attacker_name);
    } else {
        // Standardowe zachowanie dla HP, Poison itp.
        const char* v_name = Game_get_value_name(change->value_index);
        snprintf(short_source, sizeof(short_source), "%s",
                 v_name ? v_name : "???");
    }

    // 3. Składanie finalnej linii
    char line[64];
    snprintf(line, sizeof(line), "%s %s %+ld", short_receiver, short_source,
             (long)change->difference);

    GUIRenderer_set_font_size(6);
    uint8_t offset_x = is_selected ? x + 6 : x + 6;

    if (is_selected) GUIRenderer_draw_str(x, y, ">");
    GUIRenderer_draw_str(offset_x, y, line);
    if (index == Game_get_current_undo_index()) {
        GUIRenderer_draw_frame(0, y - 9, 128, 12);
    }
}

char* delegate_format_player_value(void* item, int index) {
    if (item == NULL || index < 0 || index >= NUMBER_OF_VALUES) return NULL;

    int32_t value = *(int32_t*)item;
    static char buffer[32];

    // Sprawdzamy, czy indeks mieści się w zakresie Commander Damage (20-23)
    if (index >= COMMANDER_DAMAGE_START_INDEX &&
        index < COMMANDER_DAMAGE_START_INDEX + 4) {
        int source_id = index - COMMANDER_DAMAGE_START_INDEX;
        const char* full_name = Game_get_player_name(source_id);

        char short_name[8];
        // Logika skracania: Pełne imię do 5 znaków, powyżej format XX-XX
        if (full_name) {
            size_t len = strlen(full_name);
            if (len <= 5) {
                strncpy(short_name, full_name, sizeof(short_name));
            } else {
                snprintf(short_name, sizeof(short_name), "%c%c-%c%c",
                         full_name[0], full_name[1], full_name[len - 2],
                         full_name[len - 1]);
            }
        } else {
            strcpy(short_name, "???");
        }

        // Formatowanie: "from [Name]: [Value]"
        snprintf(buffer, sizeof(buffer), "from %s: %ld", short_name,
                 (long)value);
    } else {
        // Standardowe wyświetlanie dla HP, Poison itp.
        const char* value_name = Game_get_value_name(index);
        snprintf(buffer, sizeof(buffer), "%s: %ld", value_name, (long)value);
    }

    return buffer;
}