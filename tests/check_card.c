#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <json-c/json.h>
#include <check.h>

#include "card.h"
#include "utils.h"

extern const commodity_type ALL_COMMODITIES[];
const int TOTAL_COMMODITIES = 8;

void
ck_assert_card_eq(card* card1, card* card2)
{
  ck_assert_int_eq(card1->type, card2->type);
  ck_assert_int_eq(card1->commodity, card2->commodity);
}

void
ck_assert_JSON_from_card(card_type type, commodity_type commodity)
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

  ck_assert_str_eq(card_json_str, test_str);

  json_object_put(card_json);
}

void
ck_assert_card_round_trip(card_type type, commodity_type commodity)
{
  card* start_card;
  json_object* card_json;
  card* end_card;

  start_card = card_new(type, commodity);
  card_json = JSON_from_card(start_card);
  end_card = card_from_JSON(card_json);

  json_object_put(card_json);

  ck_assert_card_eq(start_card, end_card);

  free(start_card);
  free(end_card);
}

START_TEST(test_card_commodities)
{
  ck_assert_JSON_from_card(COMMODITY, ALL_COMMODITIES[_i]);
  ck_assert_card_round_trip(COMMODITY, ALL_COMMODITIES[_i]);
}
END_TEST

START_TEST(test_card_billionaire)
{
  ck_assert_JSON_from_card(BILLIONAIRE, NONE);
  ck_assert_card_round_trip(BILLIONAIRE, NONE);
}
END_TEST

START_TEST(test_card_taxman)
{
  ck_assert_JSON_from_card(TAXMAN, NONE);
  ck_assert_card_round_trip(TAXMAN, NONE);
}
END_TEST

START_TEST(test_card_deck)
{
  size_t num_players = 8;
  bool has_billionaire = true;
  bool has_taxman = true;
  size_t deck_size = 0;

  card** deck = generate_deck(num_players, has_billionaire, has_taxman,
                              &deck_size);

  size_t* commodity_amt = calloc(TOTAL_COMMODITIES, sizeof(size_t));

  for (size_t i = 0; i < deck_size; ++i) {
    card* current_card = deck[i];
    if (current_card->commodity != NONE) {
      commodity_amt[current_card->commodity]++;
    }
  }

  /* Check that each commodity appears COMMODITY_AMOUNT times in the
     deck */
  for (int cmdty = 0; cmdty < TOTAL_COMMODITIES; ++cmdty) {
    ck_assert_int_eq(commodity_amt[cmdty], COMMODITY_AMOUNT);
  }

  free(commodity_amt);
  free_cards(deck, deck_size);
}
END_TEST

START_TEST(test_card_dealing)
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
    ck_assert_int_eq(player_hand_sizes[i], real_hand_sizes[i]);
  }

  /* Check player one's hand: */
  for (size_t i = 0; i < player_hand_sizes[0]; ++i) {
    ck_assert_card_eq(player_hands[0][i], real_player1_hand[i]);
  }

  free(player_hand_sizes);
  free_player_hands(player_hands, num_players);
  free_cards(deck, deck_size);
}
END_TEST

Suite*
card_suite(void)
{
  Suite* s;
  TCase* tc_core;
  TCase* tc_manip;

  s = suite_create("Card");

  tc_core = tcase_create("Core");

  tcase_add_loop_test(tc_core, test_card_commodities, 0, TOTAL_COMMODITIES);
  tcase_add_test(tc_core, test_card_billionaire);
  tcase_add_test(tc_core, test_card_taxman);

  tc_manip = tcase_create("Manipulation");

  tcase_add_test(tc_manip, test_card_deck);
  tcase_add_test(tc_manip, test_card_dealing);

  suite_add_tcase(s, tc_core);
  suite_add_tcase(s, tc_manip);

  return s;
}

int
main()
{
  int num_failed;
  Suite* s;
  SRunner* sr;

  s = card_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_NORMAL);
  num_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return (num_failed == 0) ? 0 : EXIT_FAILURE;
}
