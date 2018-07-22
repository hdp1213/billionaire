#ifndef _BILLIONAIRE_H_
#define _BILLIONAIRE_H_

#include <stdlib.h>
#include <stdbool.h>

#include <json-c/json.h>

#include "book.h"
#include "card_location.h"

#define MAX_PLAYERS 8
#define INITIAL_SCORE 0

struct commands {
  const char* JOIN;
  const char* START;
  const char* SUCCESSFUL_TRADE;
  const char* CANCELLED_OFFER;
  const char* BOOK_EVENT;
  const char* BILLIONAIRE;
  const char* END_GAME;
  const char* ERROR;
  const char* NEW_OFFER;
  const char* CANCEL_OFFER;
};

/**
 * Basic constructor for command objects that instantiates the "command"
 * field to whatever type is requested.
 */
json_object* make_command(const char* cmd);

/**
 * Create a JOIN command (JSON object) containing an id to identify the
 * client with.
 */
json_object* billionaire_join(char* id);

/**
 * Create a START command containing the client's hand.
 */
json_object* billionaire_start(card_location* player_hand);

/**
 * Create a SUCCESSFUL_TRADE command containing new cards and the previous
 * owner's ID.
 */
json_object* billionaire_successful_trade(offer* traded_offer);

/**
 * Create a CANCELLED_OFFER command containing a cancelled offer.
 */
json_object* billionaire_cancelled_offer(offer* cancelled_offer);

/**
 * Create a BOOK_EVENT command containing the book event.
 *
 * The participants array must have MAX_PARTICIPANTS elements in it. If
 * there is less than MAX_PARTICIPANTS participants in the event, the
 * other elements must be set to NULL.
 */
json_object* billionaire_book_event(const char* event, size_t card_amt,
                                    const char* participants[MAX_PARTICIPANTS]);

/**
 * Create a BILLIONAIRE command containing ID of winner.
 */
json_object* billionaire_billionaire(const char* winner_id);

/**
 * Create an END_GAME command.
 */
json_object* billionaire_end_game();

/**
 * Create an ERROR command containing the latest error.
 *
 * Resets cmd_errno to CMD_SUCCESS.
 */
json_object* billionaire_error();

/**
 * Get the name of a command.
 *
 * Checks cmd_errno, on failure returns NULL and sets str_len to 0.
 */
const char* get_command_name(json_object* cmd, size_t* str_len);

/**
 * Parse a string representing a JSON command list object.
 *
 * Returns a JSON array where each element is a command object.
 * Sets cmd_errno to EJSONTYPE.
 * Checks cmd_errno, on failure returns NULL.
 */
json_object* parse_command_list_string(char* json_str, size_t str_len);

/**
 * Check the type of command.
 *
 * Does not check cmd_errno. This is done by buffered_on_read() in the
 * server.
 */
bool command_is(json_object* cmd, const char* cmd_name);

#endif
