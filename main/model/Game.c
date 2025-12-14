#include "Game.h"

#include <stdio.h>  // Required for snprintf
#define NUMBER_OF_DEFAULTS 8
static Game game = {0};

void Game_init() {
    game.number_of_players = 4;

    const char* defaults[NUMBER_OF_DEFAULTS] = {
        "HP", "Poison", "Tax", "Energy", "Storm", "Cmd", "XP", "Level"};

    for (int i = 0; i < NUMBER_OF_DEFAULTS; i++) {
        snprintf(game.value_names[i], VALUE_NAME_MAX_LENGTH, "%s", defaults[i]);
    }

    for (int i = 0; i < MAX_NUMBER_OF_PLAYERS; i++) {
        snprintf(game.players[i].name, PLAYER_NAME_MAX_LENGTH, "Player %d",
                 i + 1);

        game.players[i].values[0] = 20;
        game.players[i].values[1] = 123456;
    }
}

const char* Game_get_value_name(int index) {
    if (index >= NUMBER_OF_VALUES || index < 0) {
        // log error
        return NULL;
    }
    return game.value_names[index];
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

Player* Game_get_player(int index) { return &game.players[index]; }

int32_t Game_get_commander_damage(int player_id, int source_id) {
    return game.players[player_id]
        .values[COMMANDER_DAMAGE_START_INDEX + source_id];
}

void Game_deal_commander_damage(int player_id, int source_id, int32_t amount) {
    game.players[player_id].values[COMMANDER_DAMAGE_START_INDEX + source_id] =
        (game.players[player_id]
             .values[COMMANDER_DAMAGE_START_INDEX + source_id] +
         amount) %
        999;

    // test
    game.players[player_id].values[COMMANDER_DAMAGE_START_INDEX + source_id] =
        9999;
}