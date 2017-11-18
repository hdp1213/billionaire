#ifndef _CARD_LOCATION_H_
#define _CARD_LOCATION_H_

#include <stdbool.h>
#include <stdlib.h>

#include <json-c/json.h>

typedef enum card_id card_id;
typedef struct card_location card_location;
typedef struct card_array card_array;

/**
 * Enumeration for card ID.
 *
 * Can be extended arbitrarily, as long as invalid IDs are always marked as -1.
 */
enum card_id {
  /* Invalid card */
  INVALID  = -1,
  /* Commodity cards */
  DIAMONDS = 0,
  GOLD,
  OIL,
  PROPERTY,
  MINING,
  SHIPPING,
  BANKING,
  SPORT,
  TOTAL_COMMODITY_AMOUNT,
  /* Special cards */
  BILLIONAIRE = TOTAL_COMMODITY_AMOUNT,
  TAX_COLLECTOR,
  TOTAL_UNIQUE_CARDS
};

/**
 * Struct storing an array of cards and the number of cards present in that
 * array.
 */
struct card_location {
  size_t* card_counts;
  size_t num_cards;
};

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
 * Convert a card_location struct to a JSON array of cards objects.
 */
json_object* JSON_from_card_location(card_location* card_loc);

/**
 * Convert a JSON array of cards objects to a card_location struct.
 */
card_location* card_location_from_JSON(json_object* card_loc_json);

/**
 * Create a new card_location struct that is empty and ready for card
 * insertion.
 */
card_location* card_location_new();

/**
 * Initialise a new card_location struct using variable arguments.
 *
 * The arguments given to this function must be of the card_id type.
 */
card_location* card_location_init(size_t num_cards, ...);

/**
 * Generate a set of cards to be used in a game with some number of players.
 */
card_location* generate_deck(int num_players, bool has_billionaire,
                             bool has_tax_collector);

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
 * Return the total number of cards at a card_location.
 */
size_t get_total_cards(card_location* card_loc);

/**
 * Check if the card_location contains more than amount cards of a given type.
 */
bool has_enough_cards(card_location* card_loc, card_id card, size_t amount);

/**
 * Remove all cards within a card_location struct.
 */
void clear_card_location(card_location* card_loc);

/**
 * Move an amount of a card type from source_loc to target_loc.
 */
void move_cards(card_location* from_loc, card_location* to_loc,
                card_id card, size_t amount);

/**
 * Convert a card_location into an card_array struct.
 */
card_array* flatten_card_location(card_location* card_loc);

/**
 * Free a card_location struct, and all cards within it.
 */
void free_card_location(card_location* card_loc);



/**
 * Create a new card_array with a size of num_cards
 */
card_array* card_array_new(size_t num_cards);

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
