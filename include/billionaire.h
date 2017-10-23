#ifndef _BILLIONAIRE_H_
#define _BILLIONAIRE_H_

#include <json-c/json.h>

#define MAX_PLAYERS 1

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

static int BOT_ID = 0;

json_object* make_command(const char* cmd);

json_object* billn_join();

json_object* card_to_json(card* card_obj);

const char* stringify_command(json_object* json_obj, size_t* str_len);

#endif
