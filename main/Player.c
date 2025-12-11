#include "Player.h"

#include <stdlib.h>
#include <string.h>
Player Player_create(const char* name) {
    Player player;
    memset(player.player_values, 0, sizeof(player.player_values));
    player.player_name = strdup(name);
    return player;
}
void Player_set_value(Player* player, int index, int value) {
    if (index >= 0 && index < 8) {
        player->player_values[index] = value;
    }
}
int Player_get_value(const Player* player, int index) {
    if (index >= 0 && index < 8) {
        return player->player_values[index];
    }
    return 0;
}
void Player_set_name(Player* player, const char* name) {
    free(player->player_name);
    player->player_name = strdup(name);
}
const char* Player_get_name(const Player* player) {
    return player->player_name;
}