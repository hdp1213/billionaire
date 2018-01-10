#include <assert.h>
#include <mcheck.h>
#include <stdio.h>

#include "billionaire.h"
#include "card_location.h"
#include "card_array.h"
#include "utils.h"

extern const struct commands Command;

void
mem_check_card_JSON()
{
  card_location* new_card_loc;
  json_object* card_loc_json;

  new_card_loc = card_location_init(4, DIAMONDS, DIAMONDS, DIAMONDS, OIL);
  card_loc_json = JSON_from_card_location(new_card_loc);

  free_card_location(new_card_loc);
  json_object_put(card_loc_json);
}

void
mem_check_deck_JSON()
{
  size_t num_players = 8;
  bool has_billionaire = true;
  bool has_tax_collector = true;

  const char* total_str;
  size_t total_str_len;

  card_location* deck = generate_deck(num_players, has_billionaire,
                                      has_tax_collector);

  json_object* total = json_object_new_object();
  json_object* hand = JSON_from_card_location(deck);

  json_object_object_add(total, "hand", hand);

  total_str = JSON_to_str(total, &total_str_len);

  printf("%s\n", total_str);

  free_card_location(deck);
  json_object_put(total);
}

void
mem_check_dealing()
{
  size_t num_players = 8;
  bool has_billionaire = true;
  bool has_tax_collector = true;

  card_location* deck = generate_deck(num_players, has_billionaire,
                                      has_tax_collector);

  card_array* ordered_deck = flatten_card_location(deck);
  card_location** player_hands = deal_cards(num_players, ordered_deck);

  for (size_t i = 0; i < num_players; ++i) {
    free_card_location(player_hands[i]);
  }

  free(player_hands);
  free_card_array(ordered_deck);
  free_card_location(deck);
}

void
mem_check_command_parse()
{
  char* json_str = "{\"commands\":[{\"command\":\"FINISH\"}]}";
  size_t str_len = 36;

  json_object* cmd_array = parse_command_list_string(json_str, str_len);

  JSON_ARRAY_FOREACH(cmd, cmd_array) {
    assert(command_is(cmd, Command.FINISH));
  }

  json_object_put(cmd_array);
}

int
main()
{
  mtrace();

  /* Check card memory allocation */
  mem_check_card_JSON();
  mem_check_deck_JSON();

  mem_check_dealing();

  mem_check_command_parse();
}
