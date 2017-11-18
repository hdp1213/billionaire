#ifndef _BILLIONAIRE_H_
#define _BILLIONAIRE_H_

#include <stdlib.h>
#include <stdbool.h>

#include <json-c/json.h>

#include "card_location.h"

#define MAX_PLAYERS 8

struct commands {
  const char* JOIN;
  const char* START;
  const char* SUCCESSFUL_TRADE;
  const char* BOOK_EVENT;
  const char* FINISH;
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
 * Create a FINISH command.
 */
json_object* billionaire_finish();

/**
 * Get the name of a command.
 */
const char* get_command_name(json_object* cmd, size_t* str_len);

/**
 * Check the type of command.
 */
bool command_is(json_object* cmd, const char* cmd_name);

#endif
