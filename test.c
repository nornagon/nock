#include "nock.h"

#include <stdio.h>

#define mu_assert(test, message) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; \
                              if (message) return message; } while (0)
int tests_run;

static char *test_slot() {
  mu_assert(is_eq(nock(parse("[4 0 1]")), A(4)), "[4 0 1] != 4");
  mu_assert(is_eq(nock(parse("[[2 3] 0 1]")), C(A(2), A(3))), "[[2 3] 0 1] != [2 3]");
  mu_assert(is_eq(nock(parse("[[2 3] 0 2]")), A(2)), "[[2 3] 0 2] != 2");
  mu_assert(is_eq(nock(parse("[[2 3] 0 3]")), A(3)), "[[2 3] 0 3] != 3");
  return 0;
}

static char *test_if() {
  mu_assert(is_eq(nock(parse("[42 [6 [1 1] [4 0 1] [1 233]]]")), A(233)), "if example#1 wasn't 233");
  return 0;
}

static char *test_dec() {
  mu_assert(is_eq(
        nock(parse("[42 8 [1 0] 8 [1 6 [5 [0 7] 4 0 6] [0 6] 9 2 [0 2] [4 0 6] 0 7] 9 2 0 1]")),
        A(41)
        ), "dec(42) wasn't 41");
  return 0;
}

static char *all_tests() {
  mu_run_test(test_slot);
  mu_run_test(test_if);
  mu_run_test(test_dec);
  return 0;
}

int main(int argc, char **argv) {
  char *result = all_tests();
  if (result != 0)
    printf("%s\n", result);
  else
    printf("ALL TESTS PASSED\n");
  printf("Tests run: %d\n", tests_run);

  return result != 0;
}
