#ifndef _CARD_LOCATION_H_
#define _CARD_LOCATION_H_

#include <stdbool.h>
#include <stdlib.h>

#include <json-c/json.h>

#define OFFER_MIN_CARDS 2
#define OFFER_MAX_UNIQ_COMMS 1
#define OFFER_MAX_UNIQ_WILDS 1

typedef enum card_id card_id;
typedef struct card_location card_location;

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
 * Array storing the monetary values of all cards.
 */
extern const int card_values[];

/**
 * Convert a card_location struct to a JSON array of cards objects.
 */
json_object* JSON_from_card_location(card_location* card_loc);

/**
 * Convert a JSON array of cards objects to a card_location struct.
 *
 * Sets cmd_errno to EJSONTYPE.
 * Checks cmd_errno, on failure returns NULL.
 * Note that the card's value does not need to be sent by the client.
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
 * Add an amount of cards to a card_location struct.
 */
void add_cards_to_location(card_location* card_loc, card_id card, size_t amount);

/**
 * Remove an amount of cards from a card_location struct.
 *
 * Sets cmd_errno to ECARDRM.
 */
void remove_cards_from_location(card_location* card_loc, card_id card, size_t amount);

/**
 * Merge one card location into another.
 *
 * Here, src_loc's cards are added to dest_loc, src_loc is unchanged.
 */
void merge_card_location(card_location* dest_loc, const card_location* src_loc);

/**
 * Subtract one card location's cards from another.
 *
 * Here, src_loc's cards are removed from dest_loc, src_loc is unchanged.
 * This method assumes src_loc is a subset of dest_loc, and does not
 * check for any possible overflow due to src_loc containing more cards.
 *
 * Sets cmd_errno to ECARDRM.
 */
void subtract_card_location(card_location* dest_loc, const card_location* src_loc);

/**
 * Validate an offer given by a client.
 *
 * This cross-checks against the client's hand to confirm the offer is a
 * subset, and also checks the offer contains at most one commodity type
 * and one wildcard type.
 */
void validate_offer(card_location* offer, const card_location* hand);

/**
 * Checks if a hand is a winning hand.
 */
bool has_won(const card_location* hand);

/**
 * Returns the monetary value of the hand.
 */
int evaluate_hand_score(const card_location* hand);

/**
 * Return the amount of cards of a specific type at a card_location.
 */
size_t get_card_amount(const card_location* card_loc, card_id card);

/**
 * Return the total number of cards at a card_location.
 */
size_t get_total_cards(const card_location* card_loc);

/**
 * Check if the card_location contains more than amount cards of a given type.
 */
bool has_enough_cards(const card_location* card_loc, card_id card, size_t amount);

/**
 * Remove all cards within a card_location struct.
 */
void clear_card_location(card_location* card_loc);

/**
 * Free a card_location struct, and all cards within it.
 */
void free_card_location(card_location* card_loc);

#endif
