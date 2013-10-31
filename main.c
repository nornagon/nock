#include "nock.h"

#include <stdio.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s '[42 [6 [1 0] [4 0 1] [1 233]]]'\n", argv[0]);
    exit(1);
  }
  noun *n = parse(argv[1]);
  printf("> "); print_noun(n); printf("\n");
  noun *res = nock(n);
  print_noun(res); printf("\n");
  free_noun(res);
  return 0;
}
