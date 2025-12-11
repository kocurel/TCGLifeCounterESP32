#ifndef PLAYERMODEL_H
#define PLAYERMODEL_H
typedef struct {
    int player_values[8];
    char* player_name;
} Player;

Player Player_create(const char* name);

void Player_set_value(Player* player, int index, int value);
int Player_get_value(const Player* player, int index);
void Player_set_name(Player* player, const char* name);
const char* Player_get_name(const Player* player);
#endif  // PLAYERMODEL_H