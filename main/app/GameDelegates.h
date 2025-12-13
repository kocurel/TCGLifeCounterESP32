#ifndef GAME_DELEGATES_H
#define GAME_DELEGATES_H

// Declare these so any page (Inventory, Stats, Shop) can use them
void* delegate_get_player_value(void* data_source, int index);
int delegate_get_value_count(void* data);
char* delegate_format_player_value(void* item, int index);

#endif