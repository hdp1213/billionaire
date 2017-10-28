#ifndef _GAME_STATE_H_
#define _GAME_STATE_H_

#include <stdlib.h>
#include <stdbool.h>

#include "card.h"

typedef struct game_state game_state;

struct game_state {
  /* Number of players currently in the game */
  int num_players;

  /* Number of players needed to start the game */
  int player_limit;

  /* Whether the game is currently running */
  // bool running;

  /* Size of deck */
  size_t deck_size;

  /* Deck of cards used for the game */
  card** deck;
};

/**
 * Initialise and allocate memory to the game_state structure.
 */
game_state* game_state_new(int player_limit, bool has_billionaire,
                           bool has_taxman);

/**
 * Generate cards for all players.
 */
// game_start();

/**
 * Free memory allocated to a game_state structure.
 */
void game_state_free(game_state* gs_obj);

#endif
