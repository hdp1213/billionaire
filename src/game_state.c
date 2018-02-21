#include <stdio.h>
#include <errno.h>
#include <err.h>

#include "game_state.h"
#include "card_location.h"

game_state*
game_state_new(int player_limit, bool has_billionaire, bool has_taxman)
{
  game_state* new_game_state = malloc(sizeof(game_state));

  if (new_game_state == NULL) {
    err(1, "new_game_state malloc failed");
  }

  /* Initialise parameter values */
  new_game_state->num_players = 0;
  new_game_state->player_limit = player_limit;
  new_game_state->running = false;

  /* Initialise and shuffle deck */
  card_location* unordered_deck = generate_deck(player_limit,
                                                has_billionaire,
                                                has_taxman);

  new_game_state->deck = flatten_card_location(unordered_deck);

  free_card_location(unordered_deck);

  shuffle_card_array(new_game_state->deck);

  new_game_state->current_trades = book_new();

  return new_game_state;
}

void
game_state_free(game_state* gs_obj)
{
  free_card_array(gs_obj->deck);
  free_book(gs_obj->current_trades);
  free(gs_obj);
}
