#include <check.h>
#include <stdbool.h>

#include "card_location.h"

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
  ck_assert_uint_eq(card_loc->num_cards, card_amt);

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
    ck_assert_uint_eq(card_loc->num_cards, total_cards);
  }

  /* Assert that each card only appears once */
  for (size_t card = 0; card < card_loc->num_cards; ++card) {
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
  ck_assert_uint_eq(card_loc->num_cards, card_amt - 1);

  remove_card_from_location(card_loc, OIL);
  remove_card_from_location(card_loc, OIL);
  remove_card_from_location(card_loc, OIL);

  ck_assert_uint_eq(get_card_amount(card_loc, OIL), 0);
  ck_assert_uint_eq(card_loc->num_cards, card_amt - 4);

  remove_card_from_location(card_loc, BILLIONAIRE);

  ck_assert_uint_eq(get_card_amount(card_loc, BILLIONAIRE), 0);
  ck_assert_uint_eq(card_loc->num_cards, card_amt - 5);

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
  ck_assert_uint_eq(card_loc->num_cards, card_amt);

  clear_card_location(card_loc);

  /* Assert the card_location is properly cleared */
  ck_assert_uint_eq(card_loc->num_cards, 0);

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

  ck_assert(check_card_amount(card_loc, OIL, 5) == false);

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

Suite*
card_location_suite(void)
{
  Suite* s;
  TCase* tc_core;

  s = suite_create("Card Location");

  tc_core = tcase_create("Core");

  tcase_add_test(tc_core, test_card_location_init);
  tcase_add_test(tc_core, test_card_location_add_unique);
  tcase_add_test(tc_core, test_card_location_remove);
  tcase_add_test(tc_core, test_card_location_clear);
  tcase_add_test(tc_core, test_card_location_add_remove);
  tcase_add_test(tc_core, test_card_location_move);

  suite_add_tcase(s, tc_core);

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
