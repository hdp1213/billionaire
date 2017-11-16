#ifndef _CARD_LOCATION_H_
#define _CARD_LOCATION_H_

#include <stdbool.h>
#include <stdlib.h>

#define MAX_HAND_SIZE 10

typedef enum card_id card_id;
typedef struct card_location card_location;

/**
 * Enumeration for card ID.
 *
 * Can be extended arbitrarily, as long as invalid IDs are always marked as -1.
 */
enum card_id {
  INVALID  = -1,
  DIAMONDS = 0,
  GOLD,
  OIL,
  PROPERTY,
  MINING,
  SHIPPING,
  BANKING,
  SPORT,
  BILLIONAIRE,
  TAX_COLLECTOR,
  TOTAL_UNIQUE_CARDS
};

/**
 * Struct storing an array of cards and the number of cards present in that array.
 *
 * Dynamically increases the size of cards to accomodate for new cards being
 * added.
 */
struct card_location {
  size_t* card_counts;
  size_t num_cards;
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
 * Add one card to a card_location struct.
 */
void add_card_to_location(card_location* card_loc, card_id card);

/**
 * Remove one card to a card_location struct.
 */
void remove_card_from_location(card_location* card_loc, card_id card);

/**
 * Add an amount of cards to a card_location struct.
 */
void add_cards_to_location(card_location* card_loc, card_id card, size_t amount);

/**
 * Remove an amount of cards from a card_location struct.
 */
void remove_cards_from_location(card_location* card_loc, card_id card, size_t amount);

/**
 * Return the amount of cards of a specific type at a card_location.
 */
size_t get_card_amount(card_location* card_loc, card_id card);

/**
 * Check if the card_location contains more than amount cards of a given type.
 */
bool check_card_amount(card_location* card_loc, card_id card, size_t amount);

/**
 * Remove all cards within a card_location struct.
 */
void clear_card_location(card_location* card_loc);

/**
 * Move an amount of a card type from source_loc to target_loc.
 */
void move_cards(card_location* from_loc, card_location* to_loc, card_id card, size_t amount);

/**
 * Free a card_location struct, and all cards within it.
 */
void free_card_location(card_location* card_loc);

#endif
