#include <limits.h>
#include <stdlib.h>

#include <check.h>

#include "utils.h"

const size_t g_inputs[] = {0, 1, 2, 3, 1023, 1024, 1025, 1000000, 2000000000, 2147483647};
const size_t g_outputs[] = {0, 1, 2, 4, 1024, 1024, 2048, 1048576, 2147483648, 2147483648};
const size_t g_number_amt = 10;

START_TEST(test_utils_power_of_two)
{
  ck_assert_uint_ge(g_inputs[_i], 0);
  ck_assert_uint_le(g_inputs[_i], UINT_MAX/2+1);

  ck_assert_uint_ge(g_outputs[_i], 0);
  ck_assert_uint_le(g_outputs[_i], UINT_MAX/2+1);

  ck_assert_uint_eq(get_next_highest_power_of_two(g_inputs[_i]), g_outputs[_i]);
}
END_TEST

Suite*
utils_suite(void)
{
  Suite* s;
  TCase* tc_core;

  s = suite_create("Utils");

  tc_core = tcase_create("Core");

  tcase_add_loop_test(tc_core, test_utils_power_of_two, 0, g_number_amt);

  suite_add_tcase(s, tc_core);

  return s;
}

int
main()
{
  int num_failed;
  Suite* s;
  SRunner* sr;

  s = utils_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_NORMAL);
  num_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return (num_failed == 0) ? 0 : EXIT_FAILURE;
}
