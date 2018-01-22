#include <check.h>
#include <stdbool.h>

#include "book.h"
#include "utils.h"


/* Core tests */

START_TEST(test_book_new)
{
  book* book_obj;

  book_obj = book_new();

  for (int i = 0; i < (TOTAL_COMMODITY_AMOUNT + 1) - OFFER_INDEX_OFFSET; ++i) {
    ck_assert(no_offer_at(book_obj, i));
  }

  free_book(book_obj);
}
END_TEST

START_TEST(test_book_set_offer)
{
  offer* offer_obj;
  book* book_obj;
  size_t card_amt = 3;

  offer_obj = offer_init_cards(DIAMONDS, card_amt, "test_id");

  book_obj = book_new();
  int offer_ind = offset_index(card_amt);

  ck_assert(no_offer_at(book_obj, offer_ind));

  set_offer_at(book_obj, offer_ind, offer_obj);

  ck_assert(offer_at(book_obj, offer_ind));

  free_book(book_obj);
}
END_TEST

START_TEST(test_book_fill_offer)
{
  offer* first_offer;
  offer* second_offer;
  offer* return_offer;
  book* book_obj;

  book_obj = book_new();

  first_offer = offer_init_cards(GOLD, 5, "aaaaaaa");

  return_offer = fill_offer(book_obj, first_offer);

  ck_assert(return_offer == NULL);

  second_offer = offer_init_cards(DIAMONDS, 5, "bbbbbbb");

  return_offer = fill_offer(book_obj, second_offer);
  ck_assert(no_offer_at(book_obj, 3));

  ck_assert(is_owner(return_offer, first_offer->owner_id));

  free_book(book_obj);
  free_offer(first_offer);
  free_offer(second_offer);
}
END_TEST

START_TEST(test_book_cancel_offer)
{
  offer* first_offer;
  offer* test_offer;
  book* book_obj;

  size_t card_amt = 3;
  int offer_ind = offset_index(card_amt);

  book_obj = book_new();

  first_offer = offer_init_cards(OIL, card_amt, "aaaaaaa");

  fill_offer(book_obj, first_offer);

  test_offer = cancel_offer(book_obj, card_amt, "bbbbbbb");
  ck_assert(offer_at(book_obj, offer_ind));
  ck_assert(test_offer == NULL);

  test_offer = cancel_offer(book_obj, card_amt-1, "aaaaaaa");
  ck_assert(offer_at(book_obj, offer_ind));
  ck_assert(test_offer == NULL);

  test_offer = cancel_offer(book_obj, card_amt, "aaaaaaa");
  ck_assert(no_offer_at(book_obj, offer_ind));
  ck_assert(is_owner(test_offer, first_offer->owner_id));
}
END_TEST


START_TEST(test_offer_init)
{
  offer* offer_obj;
  const char id[HASH_LENGTH] = "test_id";
  size_t card_amt = 4;

  offer_obj = offer_init_cards(DIAMONDS, card_amt, id);

  ck_assert(is_owner(offer_obj, id));
  ck_assert_int_eq(get_offer_index(offer_obj), 2);

  free_offer(offer_obj);
}
END_TEST

START_TEST(test_offer_index)
{
  for (size_t card_amt = 2; card_amt < TOTAL_COMMODITY_AMOUNT; ++card_amt) {
    offer* offer_obj;

    offer_obj = offer_init_cards(DIAMONDS, card_amt, "test_id");

    ck_assert_int_eq(get_offer_index(offer_obj), (int) card_amt - 2);

    free_offer(offer_obj);
  }
}
END_TEST

START_TEST(test_offer_json)
{
  offer* offer_obj;

  offer_obj = offer_init_cards(PROPERTY, 5, "test_id");

  json_object* offer_json;
  offer_json = JSON_from_offer(offer_obj);

  const char* offer_json_str;
  size_t str_len;
  offer_json_str = JSON_to_str(offer_json, &str_len);

  ck_assert_str_eq(offer_json_str,
                   "{\"cards\":[{\"id\":3,\"amt\":5}]}");

  free_offer(offer_obj);
  json_object_put(offer_json);
}
END_TEST

Suite*
book_suite(void)
{
  Suite* s;
  TCase* tc_core;
  TCase* tc_json;
  TCase* tc_offer;

  s = suite_create("Book");

  tc_core = tcase_create("Core");

  tcase_add_test(tc_core, test_book_new);
  tcase_add_test(tc_core, test_book_set_offer);
  tcase_add_test(tc_core, test_book_fill_offer);
  tcase_add_test(tc_core, test_book_cancel_offer);

  tc_json = tcase_create("JSON");

  tcase_add_test(tc_json, test_offer_json);

  tc_offer = tcase_create("Offer");

  tcase_add_test(tc_offer, test_offer_init);
  tcase_add_test(tc_offer, test_offer_index);

  suite_add_tcase(s, tc_core);
  suite_add_tcase(s, tc_json);
  suite_add_tcase(s, tc_offer);

  return s;
}

int
main()
{
  int num_failed;
  Suite* s;
  SRunner* sr;

  s = book_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_NORMAL);
  num_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return (num_failed == 0) ? 0 : EXIT_FAILURE;
}
