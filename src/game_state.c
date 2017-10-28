#include <stdio.h>
#include <errno.h>
#include <err.h>

#include "game_state.h"

game_state*
game_state_new(int player_limit, bool has_billionaire, bool has_taxman)
{
  game_state* new_game_state = malloc(sizeof(game_state*));

  if (new_game_state == NULL) {
    err(1, "malloc failed");
  }

  /* Initialise parameter values */
  new_game_state->num_players = 0;
  new_game_state->player_limit = player_limit;

  /* Initialise deck */
  new_game_state->deck = generate_cards(player_limit,
                                        has_billionaire,
                                        has_taxman,
                                        &new_game_state->deck_size);

  /* Shuffle the deck */
  shuffle_cards(new_game_state->deck,
                new_game_state->deck_size);

  return new_game_state;
}

void
game_state_free(game_state* gs_obj)
{
  free_cards(gs_obj->deck, gs_obj->deck_size);
  free(gs_obj);
}
