#include "GameSettings.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    int number_of_players;
    int starting_life_total;
    char* value_names[NUMBER_OF_VALUES];
} GameSettings;

static GameSettings settings;

void GameSettings_init() {
    settings.number_of_players = 4;
    settings.starting_life_total = 4000;
    const char* DEFAULT_VALUE_NAMES[NUMBER_OF_VALUES] = {
        "Life",       "Poison",  "Commander", "Energy",
        "Experience", "Custom1", "Custom2",   "Custom3"};

    for (int i = 0; i < NUMBER_OF_VALUES; i++) {
        settings.value_names[i] = strdup(DEFAULT_VALUE_NAMES[i]);
    }
}

void GameSettings_set_value_name(int index, const char* name) {
    if (index >= 0 && index < 8) {
        settings.value_names[index] = strdup(name);
    }
}
const char* GameSettings_get_value_name(int index) {
    if (index >= 0 && index < 8) {
        return settings.value_names[index];
    }
    return NULL;
}
void GameSettings_set_number_of_players(int number_of_players) {
    settings.number_of_players = number_of_players;
}
int GameSettings_get_number_of_players() { return settings.number_of_players; }
void GameSettings_set_starting_life_total(int starting_life_total) {
    starting_life_total = starting_life_total;
}
int GameSettings_get_starting_life_total() {
    return settings.starting_life_total;
}
