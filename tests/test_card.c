#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <json-c/json.h>

#include "card.h"
#include "utils.h"

extern const commodity_type ALL_COMMODITIES[];
const int TOTAL_COMMODITIES = 8;

void
test_JSON_from_card(card_type type, commodity_type commodity)
{
  card* test_card;
  json_object* card_json;
  const char* card_json_str;
  size_t card_json_str_len;
  char test_str[30];

  test_card = card_new(type, commodity);
  card_json = JSON_from_card(test_card);

  free(test_card);

  card_json_str = JSON_to_str(card_json, &card_json_str_len);
  sprintf(test_str, "{\"type\":%d,\"commodity\":%d}", type, commodity);

  free(card_json);

  assert(strcmp(card_json_str, test_str) == 0);

  free(card_json_str);
}

void
test_card_round_trip(card_type type, commodity_type commodity)
{
  card* start_card;
  json_object* card_json;
  card* end_card;

  start_card = card_new(type, commodity);
  card_json = JSON_from_card(start_card);
  end_card = card_from_JSON(card_json);

  free(card_json);

  assert(start_card->type == end_card->type);
  assert(start_card->commodity == end_card->commodity);

  free(start_card);
  free(end_card);
}

void
test_card(card_type type, commodity_type commodity)
{
  test_JSON_from_card(type, commodity);
  printf(". ");
  test_card_round_trip(type, commodity);
  printf(". ");
}

int
main()
{
  printf("Running test_card\n");

  for (int i = 0; i < TOTAL_COMMODITIES; ++i) {
    test_card(COMMODITY, ALL_COMMODITIES[i]);
  }

  test_card(BILLIONAIRE, NONE);
  test_card(TAXMAN, NONE);

  printf("\n");

  return 0;
}
