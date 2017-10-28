#ifndef _CARD_H_
#define _CARD_H_

#include <stdlib.h>
#include <stdbool.h>

#include <json-c/json.h>

#define COMMODITY_AMOUNT 9
#define MAX_HAND_SIZE 10

typedef enum card_type card_type;
typedef enum commodity_type commodity_type;
typedef struct card card;

enum card_type {
  COMMODITY   = 0,
  BILLIONAIRE = 1,
  TAXMAN      = 2
};

enum commodity_type {
  NONE     = -1,
  DIAMONDS = 0,
  GOLD     = 1,
  OIL      = 2,
  PROPERTY = 3,
  MINING   = 4,
  SHIPPING = 5,
  BANKING  = 6,
  SPORT    = 7
};

struct card {
  card_type type;
  commodity_type commodity;
};

/**
 * Convert a card structure to a json_object in preparation for sending
 * to client.
 */
json_object* card_to_JSON(card* card_obj);

/**
 * Create a new card object
 */
card* card_new(card_type type, commodity_type commodity);

/**
 * Generate a set of cards to be used in a game with a set number of
 * players.
 *
 * Also return the number of cards generated.
 */
card** generate_deck(int num_players, bool has_billionaire,
                     bool has_taxman, size_t* num_cards);

/**
 * Free memory allocated to a set of cards.
 */
void free_cards(card** cards, size_t num_cards);

/**
 * Shuffle a set of cards.
 */
void shuffle_cards(card** cards, size_t num_cards);

/**
 * Print out cards.
 */
void display_cards(card** cards, size_t num_cards);

/**
 * Deal cards to players
 */
void deal_cards(size_t num_players,
                card**** player_hands, size_t** player_hand_sizes);

#endif
