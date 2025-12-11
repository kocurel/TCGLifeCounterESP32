#ifndef GAMESETTINGSMODEL_H
#define GAMESETTINGSMODEL_H
#define NUMBER_OF_VALUES 8
void GameSettings_init();

void GameSettings_set_value_name(int index, const char* name);
const char* GameSettings_get_value_name(int index);

void GameSettings_set_number_of_players(int number_of_players);
int GameSettings_get_number_of_players();

void GameSettings_set_starting_life_total(int starting_life_total);
int GameSettings_get_starting_life_total();

#endif  // GAMESETTINGSMODEL_H