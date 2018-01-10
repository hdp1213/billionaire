#ifndef _CARD_ARRAY_H_
#define _CARD_ARRAY_H_

typedef struct card_array card_array;

#include "card_location.h"

/**
 * Struct preserving the order of cards.
 *
 * Needed for card shuffling routines, where card order is meaningful.
 */
struct card_array {
  card_id* cards;
  size_t num_cards;
};

/**
 * Create a new card_array with a size of num_cards
 */
card_array* card_array_new(size_t num_cards);

/**
 * Convert a card_location into an card_array struct.
 */
card_array* flatten_card_location(card_location* card_loc);

/**
 * Shuffle an ordered array of cards.
 */
void shuffle_card_array(card_array* card_arr);

/**
 * Deal cards to players.
 */
card_location** deal_cards(size_t num_players, card_array* ordered_deck);

/**
 * Free a card_array.
 */
void free_card_array(card_array* card_arr);

#endif
