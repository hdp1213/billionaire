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

  /* Create array used to count the number of cards, initialised to zero */
  new_card_location->card_counts = calloc(TOTAL_UNIQUE_CARDS, sizeof(size_t));

  if (new_card_location->card_counts == NULL) {
    err(1, "new_card_location->card_counts malloc failed");
  }

  /* Initialise other fields */
  new_card_location->num_cards = 0;

  return new_card_location;
}

card_location*
card_location_init(size_t num_cards, ...)
{
  card_location* new_card_location = card_location_new();

  /* Initialise card_counts with the list of new cards */
  va_list card_list;
  va_start(card_list, num_cards);

  for (size_t i = 0; i < num_cards; ++i) {
    card_id card = va_arg(card_list, card_id);

    add_cards_to_location(new_card_location, card, 1);
  }

  va_end(card_list);

  return new_card_location;
}

void
add_card_to_location(card_location* card_loc, card_id card)
{
  add_cards_to_location(card_loc, card, 1);
}

void
remove_card_from_location(card_location* card_loc, card_id card)
{
  remove_cards_from_location(card_loc, card, 1);
}

void
add_cards_to_location(card_location* card_loc, card_id card, size_t amount)
{
  card_loc->num_cards += amount;

  /* Add new cards to card_loc->card_counts */
  card_loc->card_counts[card] += amount;
}

void
remove_cards_from_location(card_location* card_loc, card_id card, size_t amount)
{
  if (check_card_amount(card_loc, card, amount)) {
    card_loc->num_cards -= amount;

    card_loc->card_counts[card] -= amount;
  }
  else {
    /* Handle not having enough cards to remove */
  }
}

size_t
get_card_amount(card_location* card_loc, card_id card)
{
  return card_loc->card_counts[card];
}

bool
check_card_amount(card_location* card_loc, card_id card, size_t amount)
{
  size_t amount_at_location = get_card_amount(card_loc, card);

  return (amount <= amount_at_location);
}

void
clear_card_location(card_location* card_loc)
{
  card_loc->num_cards = 0;

  for (size_t card = 0; card < TOTAL_UNIQUE_CARDS; ++card) {
    card_loc->card_counts[card] = 0;
  }
}

void
move_cards(card_location* from_loc, card_location* to_loc, card_id card, size_t amount)
{
  /* Only add cards to to_loc if they can be removed from from_loc */
  if (check_card_amount(from_loc, card, amount)) {
    remove_cards_from_location(from_loc, card, amount);
    add_cards_to_location(to_loc, card, amount);
  }
  else {
    /* Handle not having enough cards to remove */
  }
}

void
free_card_location(card_location* card_loc)
{
  free(card_loc->card_counts);

  /* Free the struct itself */
  free(card_loc);
}
