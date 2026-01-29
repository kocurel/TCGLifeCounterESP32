#ifndef GAME_DELEGATES_H
#define GAME_DELEGATES_H
#include <stddef.h>

#include "GUIFramework.h"

void* delegate_get_player_value(void* data_source, int index);
int delegate_get_value_count(void* data);
char* delegate_format_player_value(void* item, int index);

int delegate_get_change_history_count(void* data);
void* delegate_get_value_change(void* context, int index);

void HistoryPage_draw_item(GUIList* list, int index, uint8_t x, uint8_t y,
                           uint8_t w, uint8_t h, bool is_selected);
#endif  // GAME_DELEGATES_H