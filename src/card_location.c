#include <err.h>
#include <stdarg.h>
#include <stdio.h>

#include "card_location.h"
#include "utils.h"

card_location*
card_location_new()
{
  card_location* new_card_location = malloc(sizeof(card_location));

  if (new_card_location == NULL) {
    err(1, "new_card_location malloc failed");
  }

  /* Initialise fields of new struct sensibly */
  new_card_location->cards = NULL;
  new_card_location->num_cards = 0;
  new_card_location->fixed_size = false;
  new_card_location->store_lim = 0;

  return new_card_location;
}

card_location*
card_location_init(size_t num_cards, ...)
{
  va_list card_list;

  card_location* new_card_location = malloc(sizeof(card_location));

  if (new_card_location == NULL) {
    err(1, "new_card_location malloc failed");
  }

  new_card_location->cards = calloc(num_cards, sizeof(card*));
  new_card_location->num_cards = num_cards;
  new_card_location->fixed_size = false;
  new_card_location->store_lim = get_next_highest_power_of_two(num_cards);

  va_start(card_list, num_cards);

  for (size_t i = 0; i < num_cards; ++i) {
    new_card_location->cards[i] = va_arg(card_list, card*);
  }

  va_end(card_list);

  return new_card_location;
}

void
add_card(card_location* card_loc, card* new_card)
{
  if (card_loc->fixed_size) {
    err(1, "no cards can be added to a card_loc with fixed size");
  }

  card_loc->num_cards++;

  /* Intialise memory if we haven't already */
  if (card_loc->cards == NULL) {
    card_loc->store_lim = get_next_highest_power_of_two(card_loc->num_cards);
    card_loc->cards = calloc(card_loc->store_lim, sizeof(card*));

    if (card_loc->cards == NULL) {
      err(1, "card_loc->cards malloc failed");
    }
  }

  /* Increase storage size if the current number of cards is
     larger than the current storage limit */
  if (card_loc->num_cards > card_loc->store_lim) {
    card_loc->store_lim = get_next_highest_power_of_two(card_loc->num_cards);
    card_loc->cards = realloc(card_loc->cards, card_loc->store_lim*sizeof(card*));

    if (card_loc->cards == NULL) {
      err(1, "card_loc->cards realloc failed");
    }
  }

  /* Add new card to card_loc->cards */
  int card_ind = (int) card_loc->num_cards - 1;
  card_loc->cards[card_ind] = new_card;
}

void
clear_card_location(card_location* card_loc)
{
  if (card_loc->num_cards > 0) {
    /* Free each card */
    for (size_t i = 0; i < card_loc->num_cards; ++i) {
      free(card_loc->cards[i]);
    }

    /* Free containing array */
    free(card_loc->cards);

    /* Re-initialise defaults once again */
    card_loc->cards = NULL;
    card_loc->num_cards = 0;
    card_loc->fixed_size = false;
    card_loc->store_lim = 0;
  }
}

void
fix_card_location_size(card_location* card_loc)
{
  card_loc->fixed_size = true;

  card_loc->store_lim = card_loc->num_cards;
  card_loc->cards = realloc(card_loc->cards, card_loc->store_lim*sizeof(card*));

  if (card_loc->cards == NULL) {
    err(1, "card_loc->cards realloc failed");
  }
}

void
free_card_location(card_location* card_loc)
{
  if (card_loc->num_cards > 0) {
    /* Free each card */
    for (size_t i = 0; i < card_loc->num_cards; ++i) {
      free(card_loc->cards[i]);
    }

    /* Free containing array */
    free(card_loc->cards);
  }

  /* Free the struct itself */
  free(card_loc);
}
