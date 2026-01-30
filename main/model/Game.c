#include "Game.h"

#include <stdbool.h>
#include <stdio.h>

#include "Assert.h"
#include "Settings.h"

#define NUMBER_OF_DEFAULTS 8
Game game = {0};

void Game_init() {
    // Reset history state pointers and counters
    game.history.head = 0;
    game.history.cursor = 0;
    game.history.tail = 0;
    game.history.count = 0;

    game.number_of_players = 4;
    const char* defaults[NUMBER_OF_DEFAULTS] = {
        "HP", "Poison", "Tax", "Energy", "Storm", "Cmd", "XP", "Level"};

    // Initialize default value names
    for (int i = 0; i < NUMBER_OF_DEFAULTS; i++) {
        snprintf(game.value_names[i], VALUE_NAME_MAX_LENGTH, "%s", defaults[i]);
    }

    // Initialize players with default names and starting values
    for (int i = 0; i < MAX_NUMBER_OF_PLAYERS; i++) {
        snprintf(game.players[i].name, PLAYER_NAME_MAX_LENGTH, "Player %d",
                 i + 1);
        game.players[i].values[0] = 20;  // Default HP
        game.players[i].values[1] = 0;   // Reset poison/secondary stats
    }
}

const char* Game_get_value_name(uint8_t index) {
    ASSERT_LESSER(index, NUMBER_OF_VALUES);
    if (index >= NUMBER_OF_VALUES) return NULL;
    return game.value_names[index];
}

void History_add_change(ValueChange new_change) {
    // Branching timeline: clear redo history by setting head to current cursor
    game.history.head = game.history.cursor;
    game.history.count = game.history.head;

    // Insert new record at head
    game.history.changes[game.history.head] = new_change;

    // Calculate next head position in circular buffer
    int32_t next_head = (game.history.head + 1) % HISTORY_MAX_CAPACITY;

    // Advance tail if buffer is full and head overtakes oldest record
    if (game.history.count == HISTORY_MAX_CAPACITY &&
        next_head == game.history.tail) {
        game.history.tail = (game.history.tail + 1) % HISTORY_MAX_CAPACITY;
    }

    game.history.head = next_head;
    game.history.cursor = game.history.head;

    if (game.history.count < HISTORY_MAX_CAPACITY) {
        game.history.count++;
    }
}

void Game_set_value(int32_t new_value, uint8_t player_id, uint8_t value_id) {
    ASSERT_LESSER(player_id, MAX_NUMBER_OF_PLAYERS);
    ASSERT_LESSER(value_id, NUMBER_OF_VALUES);
    if (player_id >= MAX_NUMBER_OF_PLAYERS || value_id >= NUMBER_OF_VALUES)
        return;

    int32_t* current_value_ptr = &game.players[player_id].values[value_id];
    int32_t current_value = *current_value_ptr;

    if (new_value == current_value) return;

    // Record difference for delta-based undo/redo
    int32_t difference = new_value - current_value;
    *current_value_ptr = new_value;

    ValueChange change = {.difference = difference,
                          .player_index = player_id,
                          .value_index = value_id};

    History_add_change(change);
}

bool internal_redo_single() {
    if (game.history.cursor == game.history.head) return false;

    ValueChange* c = &game.history.changes[game.history.cursor];
    game.players[c->player_index].values[c->value_index] += c->difference;
    game.history.cursor = (game.history.cursor + 1) % HISTORY_MAX_CAPACITY;

    return true;
}

int32_t Game_get_current_undo_index() {
    if (game.history.cursor == game.history.head) return 0;
    // Calculate distance from head back to cursor
    return (game.history.head - game.history.cursor + HISTORY_MAX_CAPACITY) %
           HISTORY_MAX_CAPACITY;
}

bool internal_undo_single() {
    if (game.history.cursor == game.history.tail || game.history.count == 0)
        return false;

    game.history.cursor =
        (game.history.cursor - 1 + HISTORY_MAX_CAPACITY) % HISTORY_MAX_CAPACITY;
    ValueChange* c = &game.history.changes[game.history.cursor];
    game.players[c->player_index].values[c->value_index] -= c->difference;

    return true;
}

void Game_undo_multiple(int32_t steps) {
    ASSERT_GREATER(steps, 0);
    int32_t undos_performed = 0;
    while (undos_performed < steps) {
        if (!internal_undo_single()) break;
        undos_performed++;
    }
    printf("[HISTORY] Undid %ld steps.\n", (long)undos_performed);
}

int32_t Game_get_value(uint8_t player_id, uint8_t value_id) {
    ASSERT_LESSER(player_id, MAX_NUMBER_OF_PLAYERS);
    ASSERT_LESSER(value_id, NUMBER_OF_VALUES);
    if (player_id >= MAX_NUMBER_OF_PLAYERS || value_id >= NUMBER_OF_VALUES)
        return 0;
    return (int32_t)game.players[player_id].values[value_id];
}

Player* Game_get_player(uint8_t index) {
    ASSERT_LESSER(index, MAX_NUMBER_OF_PLAYERS);
    if (index >= MAX_NUMBER_OF_PLAYERS) return NULL;
    return &game.players[index];
}

const char* Game_get_player_name(uint8_t index) {
    ASSERT_LESSER(index, MAX_NUMBER_OF_PLAYERS);
    if (index >= MAX_NUMBER_OF_PLAYERS) return NULL;
    return game.players[index].name;
}

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
}

ChangeHistory* Game_get_change_history() { return &game.history; }

int Game_get_change_history_count() {
    return game.history.count + 1;  // Includes virtual "Game Start" state
}

ValueChange* Game_get_change(int32_t index) {
    ASSERT_LESSER_EQUAL(index, game.history.count);
    if (index < 0 || index > game.history.count) return NULL;

    // Return static empty change for the "Game Start" virtual state
    if (index == game.history.count) {
        static ValueChange zero_state = {0};
        return &zero_state;
    }

    int32_t element_index =
        (game.history.head - 1 - index + HISTORY_MAX_CAPACITY) %
        HISTORY_MAX_CAPACITY;
    return &game.history.changes[element_index];
}

void Game_undo(void) { internal_undo_single(); }
void Game_redo(void) { internal_redo_single(); }

void Game_seek_history(int32_t target_index) {
    ASSERT_LESSER_EQUAL(target_index, game.history.count);
    ASSERT_GREATER_EQUAL(target_index, 0);

    if (target_index > game.history.count || target_index < 0) return;

    int32_t current_index = Game_get_current_undo_index();

    // Iterate until cursor reaches the desired historical depth
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

void Game_set_cursor(int32_t index) { Game_seek_history(index); }

void Game_reset() {
    // Pobieramy aktualne ustawienia (np. czy gramy w YGO 8000 czy MTG 40)
    GameSettings settings = SettingsModel_get();

    for (int p = 0; p < 4; p++) {
        // 1. Reset HP (Zauważ: używamy Game_set_value, żeby reset był widoczny
        // w historii!)
        Game_set_value(settings.starting_life, p, 0);

        // 2. Reset wszystkich liczników Commander Damage dla każdego gracza
        for (int s = 0; s < 4; s++) {
            int32_t cmd_dmg_index = COMMANDER_DAMAGE_START_INDEX + s;
            Game_set_value(0, p, cmd_dmg_index);
        }
    }

    // Opcjonalnie: Jeśli nie chcemy, by reset można było "cofnąć" przez Undo:
    // Game_clear_history();

    printf("[GAME] Reset complete. New HP: %d\n", settings.starting_life);
}