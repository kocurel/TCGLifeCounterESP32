#include "Game.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "Assert.h"
#include "Settings.h"

static const char* DEFAULT_NAMES[] = {
    "HP",      "Poison",  "CmdTax", "Energy", "Storm", "XP",   "Rad",
    "Tickets", "Monarch", "City's", "Level",  "Wins",  "Turns"};

#define ACTUAL_DEFAULTS_COUNT (sizeof(DEFAULT_NAMES) / sizeof(DEFAULT_NAMES[0]))

Game game = {0};

void Game_init() {
    game.history.head = 0;
    game.history.cursor = 0;
    game.history.tail = 0;
    game.history.count = 0;

    GameSettings settings = SettingsModel_get();
    game.number_of_players = settings.player_count;

    // 1. Nazwy liczników
    for (int i = 0; i < NUMBER_OF_VALUES; i++) {
        if (i < ACTUAL_DEFAULTS_COUNT) {
            snprintf(game.value_names[i], VALUE_NAME_MAX_LENGTH, "%s",
                     DEFAULT_NAMES[i]);
        } else {
            snprintf(game.value_names[i], VALUE_NAME_MAX_LENGTH, "Val %d", i);
        }
    }

    // 2. Inicjalizacja graczy (ZMIANA: Ładowanie nazw z NVS)
    for (int i = 0; i < MAX_NUMBER_OF_PLAYERS; i++) {
        // Używamy nazwy zapisanej w ustawieniach
        snprintf(game.players[i].name, PLAYER_NAME_MAX_LENGTH, "%s",
                 settings.player_names[i]);

        for (int v = 0; v < NUMBER_OF_VALUES; v++) {
            game.players[i].values[v] = 0;
        }
    }

    Game_reset();
}

// --- Status Management ---

bool Game_is_monarch(uint8_t player_id) {
    return Game_get_value(player_id, INDEX_MONARCH) > 0;
}

void Game_set_monarch(uint8_t player_id) {
    // Monarcha jest unikalny - zabieramy innym
    for (int i = 0; i < game.number_of_players; i++) {
        if (i != player_id && Game_is_monarch(i)) {
            Game_set_value(0, i, INDEX_MONARCH);
        }
    }
    Game_set_value(1, player_id, INDEX_MONARCH);
}

void Game_toggle_blessing(uint8_t player_id) {
    int32_t current = Game_get_value(player_id, INDEX_CITY_BLESSING);
    Game_set_value(current ? 0 : 1, player_id, INDEX_CITY_BLESSING);
}

void Game_set_player_name(uint8_t player_id, const char* name) {
    if (player_id < MAX_NUMBER_OF_PLAYERS && name != NULL) {
        strncpy(game.players[player_id].name, name, PLAYER_NAME_MAX_LENGTH - 1);
        game.players[player_id].name[PLAYER_NAME_MAX_LENGTH - 1] = '\0';
    }
}

// --- Core Logic & History ---

static void History_add_change(ValueChange new_change) {
    // "Timeline branching" - usuwamy Redo jeśli robimy nową akcję
    game.history.head = game.history.cursor;
    game.history.count = game.history.head;

    game.history.changes[game.history.head] = new_change;

    int32_t next_head = (game.history.head + 1) % HISTORY_MAX_CAPACITY;
    if (game.history.count == HISTORY_MAX_CAPACITY &&
        next_head == game.history.tail) {
        game.history.tail = (game.history.tail + 1) % HISTORY_MAX_CAPACITY;
    }

    game.history.head = next_head;
    game.history.cursor = game.history.head;
    if (game.history.count < HISTORY_MAX_CAPACITY) game.history.count++;
}

void Game_set_value(int32_t new_value, uint8_t player_id, uint8_t value_id) {
    ASSERT_LESSER(player_id, MAX_NUMBER_OF_PLAYERS);
    if (player_id >= MAX_NUMBER_OF_PLAYERS || value_id >= NUMBER_OF_VALUES)
        return;

    int32_t current_value = game.players[player_id].values[value_id];
    if (new_value == current_value) return;

    int32_t difference = new_value - current_value;
    game.players[player_id].values[value_id] = new_value;

    ValueChange change = {.difference = difference,
                          .player_index = player_id,
                          .value_index = value_id};
    History_add_change(change);
}

static bool internal_redo_single() {
    if (game.history.cursor == game.history.head) return false;
    ValueChange* c = &game.history.changes[game.history.cursor];
    game.players[c->player_index].values[c->value_index] += c->difference;
    game.history.cursor = (game.history.cursor + 1) % HISTORY_MAX_CAPACITY;
    return true;
}

static bool internal_undo_single() {
    if (game.history.cursor == game.history.tail || game.history.count == 0)
        return false;
    game.history.cursor =
        (game.history.cursor - 1 + HISTORY_MAX_CAPACITY) % HISTORY_MAX_CAPACITY;
    ValueChange* c = &game.history.changes[game.history.cursor];
    game.players[c->player_index].values[c->value_index] -= c->difference;
    return true;
}

void Game_undo(void) { internal_undo_single(); }
void Game_redo(void) { internal_redo_single(); }

void Game_seek_history(int32_t target_index) {
    int32_t current_index = Game_get_current_undo_index();
    while (current_index != target_index) {
        if (current_index < target_index) {
            if (!internal_undo_single()) break;
            current_index++;
        } else {
            if (!internal_redo_single()) break;
            current_index--;
        }
    }
}

void Game_reset() {
    GameSettings settings = SettingsModel_get();
    game.number_of_players = settings.player_count;

    // Czyścimy historię przy resecie gry
    game.history.head = 0;
    game.history.cursor = 0;
    game.history.tail = 0;
    game.history.count = 0;

    for (int p = 0; p < MAX_NUMBER_OF_PLAYERS; p++) {
        for (int v = 0; v < NUMBER_OF_VALUES; v++)
            game.players[p].values[v] = 0;
        game.players[p].values[INDEX_HP] = settings.starting_life;
    }
}

// --- Getters ---

int32_t Game_get_value(uint8_t player_id, uint8_t value_id) {
    if (player_id >= MAX_NUMBER_OF_PLAYERS || value_id >= NUMBER_OF_VALUES)
        return 0;
    return game.players[player_id].values[value_id];
}

const char* Game_get_player_name(uint8_t index) {
    if (index >= MAX_NUMBER_OF_PLAYERS) return "N/A";
    return game.players[index].name;
}

Player* Game_get_player(uint8_t index) {
    return (index < MAX_NUMBER_OF_PLAYERS) ? &game.players[index] : NULL;
}

const char* Game_get_value_name(uint8_t index) {
    return (index < NUMBER_OF_VALUES) ? game.value_names[index] : "";
}

int32_t Game_get_commander_damage(int player_id, int source_id) {
    return game.players[player_id]
        .values[COMMANDER_DAMAGE_START_INDEX + source_id];
}

int32_t Game_get_current_undo_index() {
    return (game.history.head - game.history.cursor + HISTORY_MAX_CAPACITY) %
           HISTORY_MAX_CAPACITY;
}

int Game_get_change_history_count() { return game.history.count + 1; }

ValueChange* Game_get_change(int32_t index) {
    if (index < 0 || index >= game.history.count) return NULL;
    int32_t element_index =
        (game.history.head - 1 - index + HISTORY_MAX_CAPACITY) %
        HISTORY_MAX_CAPACITY;
    return &game.history.changes[element_index];
}