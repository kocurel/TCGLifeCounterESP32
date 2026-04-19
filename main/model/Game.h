#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stdint.h>

/**
 * System constraints and buffer capacities
 */
#define MAX_NUMBER_OF_PLAYERS 4
#define PLAYER_NAME_MAX_LENGTH 20
#define VALUE_NAME_MAX_LENGTH 12
#define HISTORY_MAX_CAPACITY 2000
#define NUMBER_OF_VALUES 36

/**
 * Reserved value indices within the Player values array
 */
#define INDEX_HP 0
#define INDEX_MONARCH 8
#define INDEX_CITY_BLESSING 9
#define COMMANDER_DAMAGE_START_INDEX 32

typedef struct Player Player;
typedef struct ChangeHistory ChangeHistory;
typedef struct ValueChange ValueChange;
typedef struct Game Game;

/**
 * Represents a single player state including names and tracked values
 */
struct Player {
    char name[PLAYER_NAME_MAX_LENGTH];
    int32_t values[NUMBER_OF_VALUES];
};

/**
 * Record of a single state transition for the undo/redo system
 */
struct ValueChange {
    int32_t difference;
    uint8_t player_index;
    uint8_t value_index;
};

/**
 * Circular buffer management for state history
 */
struct ChangeHistory {
    ValueChange changes[HISTORY_MAX_CAPACITY];
    int32_t cursor;
    int32_t head;
    int32_t tail;
    int32_t count;
};

/**
 * Global game state container
 */
struct Game {
    ChangeHistory history;
    Player players[MAX_NUMBER_OF_PLAYERS];
    char value_names[NUMBER_OF_VALUES][VALUE_NAME_MAX_LENGTH];
    uint8_t number_of_players;
};

/* --- Lifecycle Management --- */

/**
 * Synchronizes game state with persistent hardware settings
 */
void Game_init(void);

/**
 * Resets session values and clears history buffer
 */
void Game_reset(void);

/* --- State Modification --- */

/**
 * Updates a player value and generates a history entry
 */
void Game_set_value(int32_t value, uint8_t player_id, uint8_t value_id);

/**
 * Retrieves a specific player value by index
 */
int32_t Game_get_value(uint8_t player_id, uint8_t value_id);

/* --- Logic Helpers --- */

bool Game_is_monarch(uint8_t player_id);
void Game_set_monarch(uint8_t player_id);
void Game_toggle_blessing(uint8_t player_id);
void Game_set_player_name(uint8_t player_id, const char* name);
void Game_set_value_name(int index, const char* name);

/* --- Data Accessors --- */

Player* Game_get_player(uint8_t index);
const char* Game_get_player_name(uint8_t index);
const char* Game_get_value_name(uint8_t index);
int32_t Game_get_commander_damage(int player_id, int source_id);

/* --- History Navigation --- */

/**
 * Steps back in the state history
 */
void Game_undo(void);

/**
 * Steps forward in the state history
 */
void Game_redo(void);

/**
 * Moves history cursor to a specific relative index via multiple steps
 */
void Game_seek_history(int32_t target_index);

/**
 * Returns current relative offset in the undo buffer
 */
int32_t Game_get_current_undo_index(void);

/**
 * Returns total number of recorded transitions
 */
int Game_get_change_history_count(void);

/**
 * Retrieves a specific change record by historical index
 */
ValueChange* Game_get_change(int32_t index);

#endif