#include <check.h>
#include <stdbool.h>

#include "card_location.h"
#include "utils.h"


/* Core tests */

START_TEST(test_card_location_init)
{
  size_t card_amt = 4;
  card_location* card_loc;

  card_loc = card_location_init(card_amt,
                                DIAMONDS,
                                DIAMONDS,
                                DIAMONDS,
                                DIAMONDS);

  /* Assert number of cards in card_loc is equal to number given */
  ck_assert_uint_eq(get_total_cards(card_loc), card_amt);

  free_card_location(card_loc);
}
END_TEST

START_TEST(test_card_location_add_unique)
{
  card_id cards[] = {
    DIAMONDS,
    GOLD,
    OIL,
    PROPERTY,
    MINING,
    SHIPPING,
    BANKING,
    SPORT,
    BILLIONAIRE,
    TAX_COLLECTOR
  };
  size_t card_amt = TOTAL_UNIQUE_CARDS;

  card_location* card_loc = card_location_new();

  for (size_t i = 0; i < card_amt; ++i) {
    size_t total_cards = i + 1;
    add_card_to_location(card_loc, cards[i]);

    /* Assert amount of cards in location so far is equal to number given */
    ck_assert_uint_eq(get_total_cards(card_loc), total_cards);
  }

  /* Assert that each card only appears once */
  for (card_id card = DIAMONDS; card < TOTAL_UNIQUE_CARDS; ++card) {
    ck_assert_uint_eq(get_card_amount(card_loc, card), 1);
  }

  free_card_location(card_loc);
}
END_TEST

START_TEST(test_card_location_remove)
{
  size_t card_amt = 7;

  card_location* card_loc = card_location_init(card_amt,
                                               DIAMONDS,
                                               DIAMONDS,
                                               OIL,
                                               MINING,
                                               OIL,
                                               OIL,
                                               BILLIONAIRE);

  remove_card_from_location(card_loc, DIAMONDS);

  ck_assert_uint_eq(get_card_amount(card_loc, DIAMONDS), 1);
  ck_assert_uint_eq(get_total_cards(card_loc), card_amt - 1);

  remove_card_from_location(card_loc, OIL);
  remove_card_from_location(card_loc, OIL);
  remove_card_from_location(card_loc, OIL);

  ck_assert_uint_eq(get_card_amount(card_loc, OIL), 0);
  ck_assert_uint_eq(get_total_cards(card_loc), card_amt - 4);

  remove_card_from_location(card_loc, BILLIONAIRE);

  ck_assert_uint_eq(get_card_amount(card_loc, BILLIONAIRE), 0);
  ck_assert_uint_eq(get_total_cards(card_loc), card_amt - 5);

  free_card_location(card_loc);
}
END_TEST

START_TEST(test_card_location_clear)
{
  size_t card_amt = 5;
  card_location* card_loc;

  card_loc = card_location_init(card_amt,
                                DIAMONDS,
                                GOLD,
                                OIL,
                                PROPERTY,
                                MINING);

  /* Another sanity check */
  ck_assert_uint_eq(get_total_cards(card_loc), card_amt);

  clear_card_location(card_loc);

  /* Assert the card_location is properly cleared */
  ck_assert_uint_eq(get_total_cards(card_loc), 0);

  free_card_location(card_loc);
}
END_TEST

START_TEST(test_card_location_add_remove)
{
  size_t card_amt = 5;
  card_location* card_loc;

  card_loc = card_location_init(card_amt,
                                DIAMONDS,
                                OIL,
                                OIL,
                                DIAMONDS,
                                OIL);

  ck_assert_uint_eq(get_card_amount(card_loc, DIAMONDS), 2);
  ck_assert_uint_eq(get_card_amount(card_loc, OIL), 3);

  add_cards_to_location(card_loc, DIAMONDS, 4);

  ck_assert_uint_eq(get_card_amount(card_loc, DIAMONDS), 6);

  remove_cards_from_location(card_loc, DIAMONDS, 3);

  ck_assert_uint_eq(get_card_amount(card_loc, DIAMONDS), 3);

  remove_cards_from_location(card_loc, OIL, 3);

  ck_assert_uint_eq(get_card_amount(card_loc, OIL), 0);

  remove_cards_from_location(card_loc, OIL, 9);

  ck_assert_uint_eq(get_card_amount(card_loc, OIL), 0);

  ck_assert(has_enough_cards(card_loc, OIL, 5) == false);

  free_card_location(card_loc);
}
END_TEST

START_TEST(test_card_location_move)
{
  card_location* card_loc1;
  card_location* card_loc2;

  card_loc1 = card_location_init(6,
                                 GOLD,
                                 GOLD,
                                 DIAMONDS,
                                 GOLD,
                                 DIAMONDS,
                                 OIL);

  card_loc2 = card_location_init(6,
                                 DIAMONDS,
                                 OIL,
                                 GOLD,
                                 GOLD,
                                 DIAMONDS,
                                 OIL);

  /* Move one DIAMONDS from card_loc1 to card_loc2 */
  move_cards(card_loc1, card_loc2, DIAMONDS, 1);

  ck_assert_uint_eq(get_card_amount(card_loc1, DIAMONDS), 1);
  ck_assert_uint_eq(get_card_amount(card_loc2, DIAMONDS), 3);
  ck_assert_uint_eq(card_loc1->num_cards, 5);
  ck_assert_uint_eq(card_loc2->num_cards, 7);

  /* Move all GOLD from card_loc2 to card_loc1 */
  move_cards(card_loc2, card_loc1, GOLD, 2);

  ck_assert_uint_eq(get_card_amount(card_loc1, GOLD), 5);
  ck_assert_uint_eq(get_card_amount(card_loc2, GOLD), 0);
  ck_assert_uint_eq(card_loc1->num_cards, 7);
  ck_assert_uint_eq(card_loc2->num_cards, 5);

  /* Fail to move three OIL from card_loc1 (only one OIL) */
  move_cards(card_loc1, card_loc2, OIL, 3);

  ck_assert_uint_eq(get_card_amount(card_loc1, OIL), 1);
  ck_assert_uint_eq(get_card_amount(card_loc1, OIL), 1);
  ck_assert_uint_eq(card_loc1->num_cards, 7);
  ck_assert_uint_eq(card_loc2->num_cards, 5);

  free_card_location(card_loc1);
  free_card_location(card_loc2);
}
END_TEST

START_TEST(test_card_location_generate_deck)
{
  size_t num_players = (size_t) _i;
  card_location* deck;
  bool has_billionaire = true;
  bool has_tax_collector = true;

  deck = generate_deck(num_players, has_billionaire, has_tax_collector);

  size_t deck_size = num_players*(TOTAL_COMMODITY_AMOUNT + 1) + 2;

  ck_assert_uint_eq(get_total_cards(deck), deck_size);

  free_card_location(deck);
}
END_TEST


/* JSON tests */

START_TEST(test_card_location_json)
{
  card_location* card_loc;
  json_object* card_loc_json;

  card_loc = card_location_init(6,
                                BILLIONAIRE,
                                GOLD,
                                OIL,
                                OIL,
                                GOLD,
                                DIAMONDS);

  card_loc_json = JSON_from_card_location(card_loc);

  const char* card_loc_json_str;
  size_t str_len;

  card_loc_json_str = JSON_to_str(card_loc_json, &str_len);

  ck_assert_str_eq(card_loc_json_str,
                   "[{\"id\":0,\"amt\":1},{\"id\":1,\"amt\":2},\
{\"id\":2,\"amt\":2},{\"id\":8,\"amt\":1}]");

  free_card_location(card_loc);
  json_object_put(card_loc_json);
}
END_TEST

START_TEST(test_card_location_json_roundtrip)
{
  card_location* card_loc;
  json_object* card_loc_json;
  card_location* card_loc_after;

  size_t card_amt = 6;

  card_loc = card_location_init(card_amt,
                                BILLIONAIRE,
                                GOLD,
                                OIL,
                                OIL,
                                GOLD,
                                DIAMONDS);

  card_loc_json = JSON_from_card_location(card_loc);

  card_loc_after = card_location_from_JSON(card_loc_json);

  ck_assert_uint_eq(get_total_cards(card_loc),
                    get_total_cards(card_loc_after));

  for (card_id card = DIAMONDS; card < TOTAL_UNIQUE_CARDS; ++card) {
    ck_assert_uint_eq(get_card_amount(card_loc, card),
                      get_card_amount(card_loc_after, card));
  }

  free_card_location(card_loc);
  json_object_put(card_loc_json);
  free_card_location(card_loc_after);
}
END_TEST


/* Card array tests */

START_TEST(test_card_location_flatten)
{
  card_location* card_loc;
  size_t card_amt = 8;

  card_id cards[] = {
    OIL,
    OIL,
    PROPERTY,
    PROPERTY,
    SHIPPING,
    SPORT,
    SPORT,
    TAX_COLLECTOR
  };

  card_loc = card_location_new();

  for (size_t i = 0; i < card_amt; ++i) {
    add_card_to_location(card_loc, cards[i]);
  }

  card_array* card_arr;

  card_arr = flatten_card_location(card_loc);

  ck_assert_uint_eq(card_arr->num_cards, card_amt);

  for (size_t i = 0; i < card_amt; ++i) {
    ck_assert_uint_eq(card_arr->cards[i], cards[i]);
  }

  free_card_location(card_loc);
  free_card_array(card_arr);
}
END_TEST

START_TEST(test_card_location_dealing)
{
  const size_t real_hand_sizes[] = { 10, 10, 9, 9 };

  card_location* real_player1_hand;
  real_player1_hand = card_location_init(real_hand_sizes[0],
                                         DIAMONDS,
                                         DIAMONDS,
                                         DIAMONDS,
                                         GOLD,
                                         GOLD,
                                         OIL,
                                         OIL,
                                         PROPERTY,
                                         PROPERTY,
                                         BILLIONAIRE);

  size_t num_players = 4;
  bool has_billionaire = true;
  bool has_tax_collector = true;

  card_location* deck;
  card_array* ordered_deck;
  card_location** player_hands;

  deck = generate_deck(num_players, has_billionaire, has_tax_collector);
  ordered_deck = flatten_card_location(deck);

  player_hands = deal_cards(num_players, ordered_deck);

  for (size_t i = 0; i < num_players; ++i) {
    ck_assert_uint_eq(get_total_cards(player_hands[i]), real_hand_sizes[i]);
  }

  /* Compare hand of player 1 to expected hand */
  for (card_id card = DIAMONDS; card < TOTAL_UNIQUE_CARDS; ++card) {
    ck_assert_uint_eq(get_card_amount(player_hands[0], card),
                      get_card_amount(real_player1_hand, card));
  }

  /* Free memory */
  free_card_location(real_player1_hand);

  free_card_location(deck);
  free_card_array(ordered_deck);

  for (size_t i = 0; i < num_players; ++i) {
    free_card_location(player_hands[i]);
  }
}
END_TEST


Suite*
card_location_suite(void)
{
  Suite* s;
  TCase* tc_core;
  TCase* tc_json;
  TCase* tc_array;

  s = suite_create("Card Location");

  tc_core = tcase_create("Core");

  tcase_add_test(tc_core, test_card_location_init);
  tcase_add_test(tc_core, test_card_location_add_unique);
  tcase_add_test(tc_core, test_card_location_remove);
  tcase_add_test(tc_core, test_card_location_clear);
  tcase_add_test(tc_core, test_card_location_add_remove);
  tcase_add_test(tc_core, test_card_location_move);
  tcase_add_loop_test(tc_core, test_card_location_generate_deck, 1, 8);

  tc_json = tcase_create("JSON");

  tcase_add_test(tc_json, test_card_location_json);
  tcase_add_test(tc_json, test_card_location_json_roundtrip);

  tc_array = tcase_create("Card Arrays");

  tcase_add_test(tc_array, test_card_location_flatten);
  tcase_add_test(tc_array, test_card_location_dealing);
  // tcase_add_test(tc_array, );
  // tcase_add_test(tc_array, );
  // tcase_add_test(tc_array, );
  // tcase_add_test(tc_array, );

  suite_add_tcase(s, tc_core);
  suite_add_tcase(s, tc_json);
  suite_add_tcase(s, tc_array);

  return s;
}

int
main()
{
  int num_failed;
  Suite* s;
  SRunner* sr;

  s = card_location_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_NORMAL);
  num_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return (num_failed == 0) ? 0 : EXIT_FAILURE;
}
