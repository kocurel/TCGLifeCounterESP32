#include <stdint.h>
#define MAX_NUMBER_OF_PLAYERS 4
#define NUMBER_OF_VALUES 8
#define PLAYER_NAME_MAX_LENGTH 24
#define VALUE_NAME_MAX_LENGTH 24
#define HISTORY_MAX_CAPACITY 2000

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
    uint16_t head;
    uint16_t count;
};

struct Game {
    ChangeHistory history;
    Player players[MAX_NUMBER_OF_PLAYERS];
    char value_names[NUMBER_OF_VALUES][VALUE_NAME_MAX_LENGTH];
    uint8_t number_of_players;
};
void Game_init();
void Game_set_value(int32_t value, uint8_t player_id, uint8_t value_id);
int32_t Game_get_value(uint8_t player_id, uint8_t value_id);
