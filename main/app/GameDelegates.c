#include "GameDelegates.h"

#include <stdio.h>

#include "model/Game.h"
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
