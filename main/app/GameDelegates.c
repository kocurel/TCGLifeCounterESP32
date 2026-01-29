#include "GameDelegates.h"

#include <GUIFramework.h>
#include <stdio.h>

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

void HistoryPage_draw_item(GUIList* list, int index, uint8_t x, uint8_t y,
                           uint8_t w, uint8_t h, bool is_selected) {
    int total_count = Game_get_change_history_count();

    // Check if item is the virtual "Game Start" state at the end of the list
    if (index == total_count - 1) {
        GUIRenderer_set_font_size(6);
        GUIRenderer_draw_str(x + 10, y, "--- Game Start ---");
        return;
    }

    ValueChange* change = Game_get_change(index);
    if (!change) return;

    char line[64];
    const char* p_name = Game_get_player_name(change->player_index);
    const char* v_name = Game_get_value_name(change->value_index);

    // Format as "Player Name ValueName +Difference"
    snprintf(line, sizeof(line), "%s %s %+ld", p_name ? p_name : "???",
             v_name ? v_name : "???", (long)change->difference);

    GUIRenderer_set_font_size(6);
    GUIRenderer_draw_str(x, y, line);
}

char* delegate_format_player_value(void* item, int index) {
    if (item == NULL || index < 0 || index >= NUMBER_OF_VALUES) return NULL;

    int32_t value = *(int32_t*)item;
    const char* value_name = Game_get_value_name(index);

    static char buffer[32];  // Buffer for formatted value string
    snprintf(buffer, sizeof(buffer), "%s: %ld", value_name, (long)value);

    return buffer;
}