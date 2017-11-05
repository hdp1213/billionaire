#include <assert.h>
#include <stdbool.h>
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

  assert_card_equality(start_card, end_card);

  free(start_card);
  free(end_card);
}

void
test_card(card_type type, commodity_type commodity)
{
  test_JSON_from_card(type, commodity);
  test_card_round_trip(type, commodity);
}

void
test_card_dealing()
{
  const size_t real_hand_sizes[] = { 10, 10, 9, 9 };
  card* real_player1_hand[] = {
    card_new(COMMODITY, DIAMONDS),
    card_new(COMMODITY, DIAMONDS),
    card_new(COMMODITY, DIAMONDS),
    card_new(COMMODITY, GOLD),
    card_new(COMMODITY, GOLD),
    card_new(COMMODITY, OIL),
    card_new(COMMODITY, OIL),
    card_new(COMMODITY, PROPERTY),
    card_new(COMMODITY, PROPERTY),
    card_new(BILLIONAIRE, NONE)
  };

  size_t num_players = 4;
  bool has_billionaire = true;
  bool has_taxman = true;
  size_t deck_size = 0;

  card** deck = generate_deck(num_players, has_billionaire, has_taxman,
                              &deck_size);

  card*** player_hands;
  size_t* player_hand_sizes;

  deal_cards(num_players, deck, deck_size,
             &player_hands, &player_hand_sizes);

  for (size_t i = 0; i < num_players; ++i) {
    assert(player_hand_sizes[i] == real_hand_sizes[i]);
  }

  /* Check player one's hand: */
  for (size_t i = 0; i < player_hand_sizes[0]; ++i) {
    assert_card_equality(player_hands[0][i], real_player1_hand[i]);
  }

  free(player_hand_sizes);
  free_player_hands(player_hands, num_players);
  free_cards(deck, deck_size);
}

int
main()
{
  printf("Running test_card\n");

  printf("Testing card conversions...\n");
  for (int i = 0; i < TOTAL_COMMODITIES; ++i) {
    test_card(COMMODITY, ALL_COMMODITIES[i]);
  }

  test_card(BILLIONAIRE, NONE);
  test_card(TAXMAN, NONE);

  printf("Testing card dealing...\n");
  test_card_dealing();

  printf("Testing complete\n");

  return 0;
}
