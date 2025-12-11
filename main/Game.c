#include "Game.h"

#include "GameSettings.h"
#include "Player.h"

typedef struct {
    Player players[4];
    ChangeHistory change_history;
} Game;

static Game game;

void Game_init() {
    for (int i = 0; i < 4; i++) {
        game.players[i].player_values[0] =
            GameSettings_get_starting_life_total();
    }
    ChangeHistory_initialize(&game.change_history);
}

int32_t Game_get_player_life(int id) {
    return game.players[id].player_values[0];
}

/**
 * Retrieves the change history as a dynamically allocated array.
 * The caller is responsible for freeing the returned array using
 * @param out_count Pointer to an integer where the number of changes will be
 * stored.
 * @returns A pointer to the dynamically allocated array of ValueChange.
 */
// ValueChange* Game_get_change_history_alloc(const Game* model, int* out_count)
// {
//     return ChangeHistory_get_changes(&model->change_history, out_count);
// }
/**
 * Applies a ValueChange to the Game by updating the relevant player's
 * value. Also adds the change to the change history.
 * @param change The ValueChange to apply.
 */
// void Game_apply_ValueChange(Game* model, ValueChange change) {
// Player_set_value(&model->players.players_data[change.player_index],
//                  change.value_index, change.new_value);
// ChangeHistory_add_change(&model->change_history, change);
// }
// /**
//  * Sets the number of players in the Game's settings.
//  * @param number_of_players The new number of players to set.
//  */
// void Game_set_number_of_players(Game* model, int number_of_players) {
//     GameSettings_set_number_of_players(&model->settings, number_of_players);
// }
// /**
//  * Sets the starting life total in the Game's settings.
//  * @param starting_life_total The new starting life total to set.
//  */
// void Game_set_starting_life_total(Game* model, int starting_life_total) {
//     GameSettings_set_starting_life_total(&model->settings,
//     starting_life_total);
// }
// /**
//  * Sets the name of a value in the Game's settings.
//  * @param index The index of the value to set the name for.
//  * @param name The new name to set.
//  */
// void Game_set_value_name(Game* model, int index, const char* name) {
//     GameSettings_set_value_name(&model->settings, index, name);
// }
// /**
//  * Sets the name of a player in the Game.
//  * @param player_index The index of the player to set the name for.
//  * @param name The new name to set.
//  */
// void Game_set_player_name(Game* model, int player_index, const char* name) {
//     Player_set_name(&model->players.players_data[player_index], name);
// }
/**
 * Resets the Game with current settings.
 */
void Game_reset() {
    // gameplayers = Players_new(GameSettings_get_number_of_players());
    ChangeHistory_initialize(&game.change_history);
}
/**
 * Undoes a specified number of changes in the Game's change history.
 * @param num_changes The number of changes to undo.
 */
// void Game_undo_changes(Game* model, int num_changes) {
//     ChangeHistory_undo_changes(&model->change_history, &model->players,
//                                num_changes);
// }
/**
 * Retrieves the Players of the Game.
 */
// const Players* Game_get_Players(Game* model) { return &model->players; }