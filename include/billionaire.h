#ifndef _BILLIONAIRE_H_
#define _BILLIONAIRE_H_

#include <stdlib.h>
#include <stdbool.h>

#include <json-c/json.h>

#include "card.h"

#define MAX_PLAYERS 8

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
 * Create a START command containing the client's cards.
 */
json_object* billionaire_start(card** player_cards, size_t num_cards);

#endif
