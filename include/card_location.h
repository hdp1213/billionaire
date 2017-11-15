#ifndef _CARD_LOCATION_H_
#define _CARD_LOCATION_H_

#include <stdbool.h>
#include <stdlib.h>

#include "card.h"

#define MAX_HAND_SIZE 10

typedef struct card_location card_location;

/**
 * Struct storing an array of cards and the number of cards present in that array.
 *
 * Dynamically increases the size of cards to accomodate for new cards being
 * added.
 */
struct card_location {
  card** cards;
  size_t num_cards;

  bool fixed_size;
  size_t store_lim;
};

/**
 * Create a new card_location struct that is empty and ready for card insertion.
 *
 * All card_locations made this way do not have a fixed size.
 */
card_location* card_location_new();

/**
 * Initialise a new card_location struct using variable arguments.
 *
 * All card_locations made this way do not have a fixed size.
 */
card_location* card_location_init(size_t num_cards, ...);

/**
 * Add a card to a card_location struct.
 */
void add_card(card_location* card_loc, card* new_card);

/**
 * Remove all cards within a card_location struct.
 */
void clear_card_location(card_location* card_loc);

/**
 * Fix the size of a card_location struct to the number of cards it has.
 *
 * This operation prevents any extra cards being added. Good for deck objects.
 */
void fix_card_location_size(card_location* card_loc);

/**
 * Free a card_location struct, and all cards within it.
 */
void free_card_location(card_location* card_loc);

#endif
