#include <check.h>

#include "card.h"
#include "card_location.h"

void
ck_assert_card_eq(card* card1, card* card2)
{
  ck_assert_int_eq(card1->type, card2->type);
  ck_assert_int_eq(card1->commodity, card2->commodity);
}

START_TEST(test_card_location_init)
{
  size_t card_amt = 4;
  card_location* card_loc;

  card_loc = card_location_init(card_amt,
                                card_new(COMMODITY, DIAMONDS),
                                card_new(COMMODITY, DIAMONDS),
                                card_new(COMMODITY, DIAMONDS),
                                card_new(COMMODITY, DIAMONDS));

  /* Check number of cards in card_loc is equal to number given */
  ck_assert_uint_eq(card_loc->num_cards, card_amt);

  /* Check store limit is == to number of cards given (4) */
  ck_assert_uint_eq(card_loc->store_lim, card_amt);

  free_card_location(card_loc);
}
END_TEST

START_TEST(test_card_location_add)
{
  card* cards[] = {
    card_new(COMMODITY, DIAMONDS),
    card_new(COMMODITY, GOLD),
    card_new(COMMODITY, OIL),
    card_new(COMMODITY, PROPERTY),
    card_new(COMMODITY, MINING),
    card_new(COMMODITY, SHIPPING),
    card_new(COMMODITY, BANKING),
    card_new(COMMODITY, SPORT),
    card_new(BILLIONAIRE, NONE),
    card_new(TAXMAN, NONE)
  };
  size_t card_amt = 10;

  card_location* card_loc = card_location_new();

  for (size_t i = 0; i < card_amt; ++i) {
    add_card(card_loc, cards[i]);

    /* Check store limit is >= amount of cards in location so far */
    ck_assert_uint_ge(card_loc->store_lim, card_loc->num_cards);
  }

  /* Check number of cards in card_loc is equal to number given */
  ck_assert_uint_eq(card_loc->num_cards, card_amt);

  /* Check store limit is >= to number of cards given */
  ck_assert_uint_ge(card_loc->store_lim, card_amt);

  /* Check that cards are actually the same */
  for (size_t i = 0; i < card_loc->num_cards; ++i) {
    ck_assert_card_eq(card_loc->cards[i], cards[i]);
  }

  /* Calling this should also free cards initialised in cards[] */
  free_card_location(card_loc);
}
END_TEST

START_TEST(test_card_location_clear)
{
  size_t card_amt = 4;
  card_location* card_loc;

  card_loc = card_location_init(card_amt,
                                card_new(COMMODITY, DIAMONDS),
                                card_new(COMMODITY, GOLD),
                                card_new(COMMODITY, OIL),
                                card_new(COMMODITY, PROPERTY));

  ck_assert(card_loc->cards != NULL);
  ck_assert_uint_ne(card_loc->num_cards, 0);
  ck_assert_uint_ne(card_loc->store_lim, 0);

  clear_card_location(card_loc);

  ck_assert(card_loc->cards == NULL);
  ck_assert_uint_eq(card_loc->num_cards, 0);
  ck_assert_uint_eq(card_loc->store_lim, 0);
}
END_TEST

START_TEST(test_card_location_fix_size)
{
  size_t card_amt = 6;
  card_location* card_loc;

  card_loc = card_location_init(card_amt,
                                card_new(COMMODITY, DIAMONDS),
                                card_new(COMMODITY, DIAMONDS),
                                card_new(COMMODITY, DIAMONDS),
                                card_new(COMMODITY, DIAMONDS),
                                card_new(COMMODITY, GOLD),
                                card_new(BILLIONAIRE, NONE));

  /* Check store limit is > to number of cards given (8 > 6) */
  ck_assert_uint_gt(card_loc->store_lim, card_amt);

  fix_card_location_size(card_loc);

  /* Check store limit == number of cards after size fix */
  ck_assert_uint_eq(card_loc->store_lim, card_amt);

  free_card_location(card_loc);
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
  tcase_add_test(tc_core, test_card_location_add);
  tcase_add_test(tc_core, test_card_location_clear);
  tcase_add_test(tc_core, test_card_location_fix_size);

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
