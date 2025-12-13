#include "Game.h"

#include <stdio.h>  // Required for snprintf

static Game game = {0};

void Game_init() {
    game.number_of_players = 4;

    const char* defaults[NUMBER_OF_VALUES] = {"Health",  "Mana",  "Strength",
                                              "Defense", "Speed", "Gold",
                                              "XP",      "Level"};

    for (int i = 0; i < NUMBER_OF_VALUES; i++) {
        snprintf(game.value_names[i], VALUE_NAME_MAX_LENGTH, "%s", defaults[i]);
    }

    for (int i = 0; i < MAX_NUMBER_OF_PLAYERS; i++) {
        snprintf(game.players[i].name, PLAYER_NAME_MAX_LENGTH, "Player %d",
                 i + 1);

        game.players[i].values[0] = 20;
    }
}

const char* Game_get_value_name(int index) {
    if (index >= NUMBER_OF_VALUES || index < 0) {
        // log error
        return NULL;
    }
    return game.value_names[index];
}

void* delegate_get_player_value(void* data_source, int index) {
    struct Player* p = (Player*)data_source;
    if (index < 0 || index >= NUMBER_OF_VALUES) {
        // log error
        return NULL;
    }
    return &p->values[index];
}

int delegate_get_value_count() { return NUMBER_OF_VALUES; }

char* delegate_format_value(void* item, int index) {
    if (item == NULL || index < 0 || index >= NUMBER_OF_VALUES) {
        // log error
        return NULL;
    }
    int32_t* value = (int32_t*)item;
    const char* value_name = Game_get_value_name(index);
    static char buffer[VALUE_NAME_MAX_LENGTH + 48];
    sprintf(buffer, "%s: %ld", value_name, *value);
    return buffer;
}

void History_add_change(ValueChange new_change) {
    game.history.changes[game.history.head] = new_change;

    game.history.head = (game.history.head + 1) % HISTORY_MAX_CAPACITY;

    if (game.history.count < HISTORY_MAX_CAPACITY) {
        game.history.count++;
    }
}

void Game_set_value(int32_t new_value, uint8_t player_id, uint8_t value_id) {
    if (player_id >= MAX_NUMBER_OF_PLAYERS || value_id >= NUMBER_OF_VALUES) {
        // Log error and return early
        return;
    }
    int32_t* current_value_ptr = &game.players[player_id].values[value_id];
    int32_t current_value = *current_value_ptr;

    if (new_value == current_value) {
        return;
    }

    int32_t difference = new_value - current_value;

    *current_value_ptr = new_value;

    ValueChange change = {.difference = difference,
                          .player_index = player_id,
                          .value_index = value_id};

    History_add_change(change);
}

int32_t Game_get_value(uint8_t player_id, uint8_t value_id) {
    // 1. Validation (Safety check for both parameters)
    if (player_id >= MAX_NUMBER_OF_PLAYERS || value_id >= NUMBER_OF_VALUES) {
        // Log error (or use your Debug.h trace macro) here
        // Return a safe default (0) to prevent crashing the caller.
        return 0;
    }
    // 2. Access the value
    return (int32_t)game.players[player_id].values[value_id];
}