#ifndef _BILLIONAIRE_H_
#define _BILLIONAIRE_H_

#include <json-c/json.h>

#define MAX_PLAYERS 8

typedef enum card_type card_type;
typedef enum commodity_type commodity_type;
typedef struct card card;

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

enum card_type {
  COMMODITY,
  BILLIONAIRE,
  TAXMAN
};

struct card {
  commodity_type commodity;
  card_type type;
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

/**
 * Basic constructor for command objects that instantiates the "command"
 * field to whatever type is requested.
 */
json_object* make_command(const char* cmd);

/**
 * Create a JOIN command (JSON object), and generate a unique client id
 * based on its address.
 *
 * The id is written to the input variable id
 */
json_object* billionaire_join(const char* addr, size_t length, char** id);

/**
 * Convert a card structure to a json_object in preparation for sending
 * to client.
 */
json_object* card_to_json(card* card_obj);

/**
 * Turn a json_object into a string and return both the string and its
 * length.
 */
const char* stringify_command(json_object* json_obj, size_t* str_len);

#endif
