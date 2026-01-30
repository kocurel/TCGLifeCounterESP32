#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_NUMBER_OF_PLAYERS 4
#define PLAYER_NAME_MAX_LENGTH 20
#define VALUE_NAME_MAX_LENGTH 12
#define HISTORY_MAX_CAPACITY 2000

#define NUMBER_OF_VALUES 32
#define COMMANDER_DAMAGE_START_INDEX 20

// Forward declarations for cleaner struct usage
typedef struct Player Player;
typedef struct ChangeHistory ChangeHistory;
typedef struct ValueChange ValueChange;
typedef struct Game Game;

struct Player {
    char name[PLAYER_NAME_MAX_LENGTH];
    int32_t values[NUMBER_OF_VALUES];
};

struct ValueChange {
    int32_t difference;
    uint8_t player_index;
    uint8_t value_index;
};

struct ChangeHistory {
    ValueChange changes[HISTORY_MAX_CAPACITY];
    int32_t cursor;  // Current point in time
    int32_t head;    // Newest point in time (future boundary)
    int32_t tail;    // Oldest point in time (past boundary)
    int32_t count;   // Total undoable/redoable entries
};

struct Game {
    ChangeHistory history;
    Player players[MAX_NUMBER_OF_PLAYERS];
    char value_names[NUMBER_OF_VALUES][VALUE_NAME_MAX_LENGTH];
    uint8_t number_of_players;
};

/* --- Core Game Logic --- */
void Game_init(void);
void Game_set_value(int32_t value, uint8_t player_id, uint8_t value_id);
int32_t Game_get_value(uint8_t player_id, uint8_t value_id);
void Game_reset();
/* --- Getters --- */
Player* Game_get_player(uint8_t index);
const char* Game_get_player_name(uint8_t index);
const char* Game_get_value_name(uint8_t index);
int32_t Game_get_commander_damage(int player_id, int source_id);

/* --- History & Undo/Redo --- */
void Game_undo(void);
void Game_redo(void);
void Game_undo_multiple(int32_t steps);
void Game_seek_history(int32_t target_index);
int32_t Game_get_current_undo_index(void);

/* --- Internal History Access (for UI/Tests) --- */
ChangeHistory* Game_get_change_history(void);
int Game_get_change_history_count(void);
ValueChange* Game_get_change(int32_t index);

void Game_deal_commander_damage(int player_id, int source_id, int32_t amount);

#endif  // GAME_H