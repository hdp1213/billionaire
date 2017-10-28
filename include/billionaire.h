#ifndef _BILLIONAIRE_H_
#define _BILLIONAIRE_H_

#include <stdbool.h>

#include <json-c/json.h>

#define MAX_PLAYERS 8
#define COMMODITY_AMOUNT 9
#define MAX_HAND_SIZE 10

typedef enum card_type card_type;
typedef enum commodity_type commodity_type;
typedef struct card card;

typedef struct game_state game_state;

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

struct commands {
  const char* JOIN;
  const char* START;
  const char* RECEIVE;
  const char* CHECK;
  const char* FINISH;
  const char* ASK;
  const char* CANCEL;
  const char* BILLIONAIRE;
};

struct game_state {
  /* Number of players currently in the game */
  int num_players;

  /* Number of players needed to start the game */
  int player_limit;

  /* Whether the game is currently running */
  // bool running;

  /* Size of deck */
  size_t deck_size;

  /* Deck of cards used for the game */
  card** deck;
};

/**
 * Basic constructor for command objects that instantiates the "command"
 * field to whatever type is requested.
 */
json_object* make_command(const char* cmd);

/**
 * Create a JOIN command (JSON object), and generate a unique client id
 * based on its address.
 *
 * The id is written to the input variable id.
 */
json_object* billionaire_join(const char* addr, size_t length, char** id);

json_object* billionaire_start(card** player_cards, size_t num_cards);

/**
 * Convert a card structure to a json_object in preparation for sending
 * to client.
 */
json_object* card_to_json(card* card_obj);

/**
 * Generate a set of cards to be used in a game with a set number of
 * players.
 *
 * Also return the number of cards generated.
 */
card** generate_cards(int num_players, bool has_billionaire,
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
 * Initialise and allocate memory to the game_state structure.
 */
game_state* game_state_new(int player_limit, bool has_billionaire,
                           bool has_taxman);

/**
 * Generate cards for all players.
 */
// game_start();

/**
 * Free memory allocated to a game_state structure.
 */
void game_state_free(game_state* gs_obj);

/**
 * Turn a json_object into a string and return both the string and its
 * length.
 */
const char* stringify_command(json_object* json_obj, size_t* str_len);

#endif
