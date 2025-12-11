#ifndef GAMEMODEL_H
#define GAMEMODEL_H
#include "ChangeHistory.h"
#include "GameSettings.h"

/**
 * Initializes the Game with the provided settings.
 * @param game The Game to initialize.
 * @param settings The GameSettings to apply.
 */
void Game_init();

int32_t Game_get_player_life(int id);
/**
 * Retrieves the change history as a dynamically allocated array.
 * The caller is responsible for freeing the returned array using free().
 * @param out_count Pointer to an integer where the number of changes will be
 * stored.
 */
ValueChange* Game_get_change_history_alloc(int* out_count);
/**
 * Applies a ValueChange to the Game by updating the relevant player's
 * value. Also adds the change to the change history.
 * @param change The ValueChange to apply.
 */
void Game_apply_ValueChange(ValueChange change);
/**
 * Sets the number of players in the Game's settings.
 * @param number_of_players The new number of players to set.
 */
void Game_set_number_of_players(int number_of_players);
/**
 * Sets the starting life total in the Game's settings.
 * @param starting_life_total The new starting life total to set.
 */
void Game_set_starting_life_total(int starting_life_total);
/**
 * Sets the name of a value in the Game's settings
 * @param index The index of the value to set the name for.
 * @param name The new name to set.
 */
void Game_set_value_name(int index, const char* name);
/**
 * Sets the name of a player in the Game.
 * @param player_index The index of the player to set the name for.
 * @param name The new name to set.
 */
void Game_set_player_name(int player_index, const char* name);
/**
 * Resets the Game to its initial state based on current settings.
 */
void Game_reset();
/**
 * Undoes a specified number of changes in the Game's change history.
 * @param num_changes The number of changes to undo.
 */
void Game_undo_changes(int num_changes);
/**
 * Retrieves the Players of the Game.
 */
// const Players* Game_get_Players(model);
#endif  // GAMEMODEL_H