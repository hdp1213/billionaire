#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <json-c/json.h>

#include "card.h"
#include "utils.h"

void
test_card_to_JSON(card_type type, commodity_type commodity)
{
  const char* card_json_str;
  size_t card_json_str_len;

  char test_str[100];

  card* test_card = card_new(type, commodity);

  json_object* card_json = card_to_JSON(test_card);

  card_json_str = JSON_to_str(card_json, &card_json_str_len);

  sprintf(test_str, "{\"type\":%d,\"commodity\":%d}", type, commodity);

  printf("%s == %s ? ", card_json_str, test_str);
  assert(strcmp(card_json_str, test_str) == 0);
  printf("y\n");
}

int
main(int argc, char** argv)
{
  test_card_to_JSON(COMMODITY, GOLD);
  return 0;
}
