#ifndef _GAME_STATE_H_
#define _GAME_STATE_H_

#include <stdlib.h>
#include <stdbool.h>

#include "card_array.h"
#include "book.h"

typedef struct game_state game_state;

struct game_state {
  /* Number of players currently in the game */
  int num_players;

  /* Number of players needed to start the game */
  int player_limit;

  /* Whether the game is currently running */
  bool running;

  /* Deck of cards used for the game */
  card_array* deck;

  /* Trade book containing active offers */
  book* current_trades;
};

/**
 * Global game state structure.
 */
extern game_state* billionaire_game;

/**
 * Initialise and allocate memory to the game_state structure.
 */
game_state* game_state_new(int player_limit, bool has_billionaire,
                           bool has_taxman);

/**
 * Check whether a game_state has reached the player limit.
 *
 * This will return false if there are too many players.
 */
bool is_full(const game_state* gs_obj);

/**
 * Check whether a game_state is running.
 */
bool is_running(const game_state* gs_obj);

/**
 * Generate cards for all players.
 */
// game_start();

/**
 * Free memory allocated to a game_state structure.
 */
void game_state_free(game_state* gs_obj);

#endif
