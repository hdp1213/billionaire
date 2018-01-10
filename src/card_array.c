#include <assert.h>
#include <err.h>
#include <stdio.h>

#include "card_array.h"

card_array*
card_array_new(size_t num_cards)
{
  card_array* new_card_array = malloc(sizeof(card_array));

  if (new_card_array == NULL) {
    err(1, "new_card_array malloc failed");
  }

  new_card_array->cards = malloc(num_cards*sizeof(card_id));

  if (new_card_array->cards == NULL) {
    err(1, "new_card_array->cards malloc failed");
  }

  new_card_array->num_cards = num_cards;

  return new_card_array;
}

card_array*
flatten_card_location(card_location* card_loc)
{
  size_t total_card_amt = get_total_cards(card_loc);

  card_array* card_arr = card_array_new(total_card_amt);

  size_t i = 0;

  for (card_id card = DIAMONDS; card < TOTAL_UNIQUE_CARDS; ++card) {
    size_t card_amt = get_card_amount(card_loc, card);

    if (card_amt == 0) {
      continue;
    }

    for (size_t card_number = 0; card_number < card_amt; ++card_number) {
      assert(i < total_card_amt);
      card_arr->cards[i] = card;
      i++;
    }
  }

  return card_arr;
}

void
shuffle_card_array(card_array* card_arr)
{
  if (card_arr->num_cards > 1) {
    for (size_t i = 0; i < card_arr->num_cards - 1; ++i) {
      size_t rnd = (size_t) rand();
      size_t j = i + rnd/(RAND_MAX/(card_arr->num_cards - i) + 1);

      card_id tmp = card_arr->cards[j];
      card_arr->cards[j] = card_arr->cards[i];
      card_arr->cards[i] = tmp;
    }
  }
}

card_location**
deal_cards(size_t num_players, card_array* ordered_deck)
{
  card_location** player_hands = calloc(num_players, sizeof(card_location*));

  if (player_hands == NULL) {
    err(1, "*player_hands malloc failed");
  }

  for (size_t i = 0; i < num_players; ++i) {
    player_hands[i] = card_location_new();
  }

  /* Deal cards out */
  for (size_t i = 0; i < ordered_deck->num_cards; ++i) {
    /* Get the current player */
    size_t iplayer = i % num_players;

#ifdef DBUG
    printf("Player %zu, card %zu\n", iplayer, i);
#endif /* DBUG */

    /* Give a card from the deck to the current player */
    add_card_to_location(player_hands[iplayer], ordered_deck->cards[i]);
  }

  return player_hands;
}

void
free_card_array(card_array* card_arr)
{
  free(card_arr->cards);
  free(card_arr);
}
