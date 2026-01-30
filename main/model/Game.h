#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_NUMBER_OF_PLAYERS 4
#define PLAYER_NAME_MAX_LENGTH 20
#define VALUE_NAME_MAX_LENGTH 12
#define HISTORY_MAX_CAPACITY 2000

#define NUMBER_OF_VALUES 32

// Indeksy specjalne w tablicy values[]
#define INDEX_HP 0
#define INDEX_MONARCH 8
#define INDEX_CITY_BLESSING 9
#define COMMANDER_DAMAGE_START_INDEX 20

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
    int32_t cursor;
    int32_t head;
    int32_t tail;
    int32_t count;
};

struct Game {
    ChangeHistory history;
    Player players[MAX_NUMBER_OF_PLAYERS];
    char value_names[NUMBER_OF_VALUES][VALUE_NAME_MAX_LENGTH];
    uint8_t number_of_players;
};

/* --- Core Logic --- */
void Game_init(void);
void Game_reset(void);
void Game_set_value(int32_t value, uint8_t player_id, uint8_t value_id);
int32_t Game_get_value(uint8_t player_id, uint8_t value_id);

/* --- Status Helpers --- */
bool Game_is_monarch(uint8_t player_id);
void Game_set_monarch(uint8_t player_id);
void Game_toggle_blessing(uint8_t player_id);
void Game_set_player_name(uint8_t player_id, const char* name);

/* --- Getters --- */
Player* Game_get_player(uint8_t index);
const char* Game_get_player_name(uint8_t index);
const char* Game_get_value_name(uint8_t index);
int32_t Game_get_commander_damage(int player_id, int source_id);
void Game_deal_commander_damage(int player_id, int source_id, int32_t amount);

/* --- History & Undo/Redo --- */
void Game_undo(void);
void Game_redo(void);
void Game_seek_history(int32_t target_index);
int32_t Game_get_current_undo_index(void);
int Game_get_change_history_count(void);
ValueChange* Game_get_change(int32_t index);

#endif