#include "GameDelegates.h"

#include <GUIFramework.h>
#include <stdio.h>

#include "model/Game.h"

extern Game game;

void* delegate_get_player_value(void* data_source, int index) {
    struct Player* p = (Player*)data_source;
    if (index < 0 || index >= NUMBER_OF_VALUES) {
        // log error
        return NULL;
    }
    return &p->values[index];
}

int delegate_get_value_count(void* data) { return NUMBER_OF_VALUES; }

char* delegate_format_player_value(void* item, int index) {
    if (item == NULL || index < 0 || index >= NUMBER_OF_VALUES) {
        // log error
        return NULL;
    }
    int32_t* value_ptr = (int32_t*)item;
    int32_t value = *value_ptr;
    const char* value_name = Game_get_value_name(index);
    static char buffer[VALUE_NAME_MAX_LENGTH + 12];
    sprintf(buffer, "%s: %ld", value_name, value);
    return buffer;
}

int delegate_get_change_history_count(void* data) {
    return Game_get_change_history_count();
}

void* delegate_get_value_change(void* context, int index) {
    return Game_get_change(index);
}

void HistoryPage_draw_item(GUIList* list, int index, uint8_t x, uint8_t y,
                           uint8_t w, uint8_t h, bool is_selected) {
    // 1. Get the Data
    ValueChange* change = Game_get_change(index);
    if (!change) return;
    GUI_TRACE("HistoryPage_draw_item",
              "Drawing item at index %d, position %d, %d", index, x, y);
    char line1[32];
    GUIRenderer_set_font_size(6);
    snprintf(line1, 48, "%s, %s: %ld",
             Game_get_player_name(change->value_index),
             Game_get_value_name(change->value_index), change->difference);
    GUIRenderer_draw_str(x, y, line1);

    // char line2[16];
    // snprintf(line2, 16, "%+ld",
    // );  // %+ld adds the '+' sign automatically
    // GUIRenderer_draw_str(x + 2, y + 9,
    //                      line2);  // Offset Y by 8 pixels for next line
}