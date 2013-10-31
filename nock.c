#include "nock.h"

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

void free_noun(noun* n) {
  if (n && is_cell(n)) { free_noun(n->as_cell.a); free_noun(n->as_cell.b); } free(n); }
#define NUMARGS(...)  (sizeof((noun*[]){__VA_ARGS__})/sizeof(noun*))
#define L(...) (list(NUMARGS(__VA_ARGS__), __VA_ARGS__))
noun* list(int numargs, ...) {
  noun *root, *cell = root = C(NULL, NULL), *prev = NULL;
  va_list ap;
  va_start(ap, numargs);
  while (numargs--) {
    noun *n = va_arg(ap, noun*);
    if (fst(cell) == NULL) fst(cell) = n;
    else { prev = cell; cell = snd(cell) = C(n, NULL); }
  }
  if (prev) { snd(prev) = fst(cell); free(cell); }
  va_end(ap);
  return root;
}

noun* copy_noun(noun* n) {
  if (is_cell(n)) return C(copy_noun(fst(n)), copy_noun(snd(n)));
  else return A(val(n));
}

void print_noun(noun* n) {
  if (is_atom(n)) printf("%lu", val(n));
  else { printf("["); print_noun(fst(n)); printf(" "); print_noun(snd(n)); printf("]"); }
}

void __attribute__ ((noreturn)) crash(const char* message) {
  fprintf(stderr, "CRASH: %s\n", message);
  exit(1);
}

noun* inc(noun* n) {
  // 6  ::    +[a b]            +[a b]
  if (is_cell(n)) return inc(n);
  // 7  ::    +a                1 + a
  val(n) += 1; return n;
}
int is_eq(noun* a, noun* b) {
  if (is_atom(a) != is_atom(b)) return 0;
  if (is_atom(a))
    return val(a) == val(b);
  else if (is_cell(a))
    return is_eq(fst(a), fst(b)) && is_eq(snd(a), snd(b));
  return 0;
}
noun* eq(noun* n) {
  // 8  ::    =[a a]            0
  // 9  ::    =[a b]            1
  if (is_cell(n)) {
    noun *a = fst(n), *b = snd(n);
    int r = is_eq(a, b);
    free_noun(n);
    return A(r ? 0 : 1);
  }
  // 10 ::    =a                =a	
  return eq(n);
}
noun* slot(noun* n) {
  printf("/"); print_noun(n); printf("\n");
  if (is_cell(n)) {
    if (is_atom(fst(n))) {
      atom_t idx = val(fst(n));
      noun* subj = snd(n);
      free(fst(n)); free(n);
      // 12 ::    /[1 a]            a
      if          (idx == 1)        { return subj; }
      // 13 ::    /[2 a b]          a
      if (idx == 2 && is_cell(subj)) { noun* r = fst(subj); free_noun(snd(subj)); free(subj); return r; }
      // 14 ::    /[3 a b]          b
      if (idx == 3 && is_cell(subj)) { noun* r = snd(subj); free_noun(fst(subj)); free(subj); return r; }
      // 15 ::    /[(a + a) b]      /[2 /[a b]]
      if          (idx % 2 == 0)    { return slot(C(A(2), slot(C(A(idx/2), subj)))); }
      // 16 ::    /[(a + a + 1) b]  /[3 /[a b]]
      if          (idx % 2 == 1)    { return slot(C(A(3), slot(C(A(idx/2), subj)))); }
      return slot(A(0)); // TODO proper crash
    }
  }
  // 17 ::    /a                /a
  return slot(n);
}
noun* nock(noun* n) {
  // contract: by calling this function, you release ownership of |n| and all
  // its children. the return value belongs to you. take good care of it.
  printf("*"); print_noun(n); printf("\n");
  if (is_atom(n)) return n;
  noun* formula = snd(n);
  noun* subj = fst(n);
  free(n);
  // 19 ::    *[a [b c] d]      [*[a b c] *[a d]]
  if (is_cell(fst(formula))) {
    noun *a = subj, *b = fst(fst(formula)), *c = snd(fst(formula)), *d = snd(formula);
    noun *a2 = copy_noun(a);
    free(fst(formula)); free(formula);
    return C(nock(C(a, C(b, c))), nock(C(a2, d)));
  } else {
    atom_t instruction = val(fst(formula));
    free(fst(formula));
    noun* arg = snd(formula);
    free(formula);
    // 21 ::    *[a 0 b]          /[b a]
    if (instruction == 0) {
      noun *a = subj, *b = arg;
      return slot(C(b, a));
    }
    // 22 ::    *[a 1 b]          b
    if (instruction == 1) {
      free_noun(subj);
      return arg;
    }
    // 23 ::    *[a 2 b c]        *[*[a b] *[a c]]
    if (instruction == 2 && is_cell(arg)) {
      noun *a = subj, *b = fst(arg), *c = snd(arg); free(arg);
      noun *a2 = copy_noun(a);
      return nock(C(nock(C(a, b)), nock(C(a2, c))));
    }
    // 24 ::    *[a 3 b]          ?*[a b]
    if (instruction == 3) {
      noun *tmp = nock(C(subj, arg));
      noun *r = is_cell(tmp) ? A(0) : A(1);
      free_noun(tmp);
      return r;
    }
    // 25 ::    *[a 4 b]          +*[a b]
    if (instruction == 4) {
      return inc(nock(C(subj, arg)));
    }
    // 26 ::    *[a 5 b]          =*[a b]
    if (instruction == 5) {
      return eq(nock(C(subj, arg)));
    }
    // 28 ::    *[a 6 b c d]      *[a 2 [0 1] 2 [1 c d] [1 0] 2 [1 2 3] [1 0] 4 4 b]
    if (instruction == 6 && is_cell(arg) && is_cell(snd(arg))) {
      noun *a = subj, *b = fst(arg), *c = fst(snd(arg)), *d = snd(snd(arg));
      free(snd(arg)); free(arg);
      return nock(L(a, A(2), L(A(0), A(1)), A(2), L(A(1), c, d), L(A(1), A(0)), A(2), L(A(1), A(2), A(3)), L(A(1), A(0)), A(4), A(4), b));
    }
    // 29 ::    *[a 7 b c]        *[a 2 b 1 c]
    if (instruction == 7 && is_cell(arg)) {
      noun *a = subj, *b = fst(arg), *c = snd(arg); free(arg);
      return nock(L(a, A(2), b, A(1), c));
    }
    // 30 ::    *[a 8 b c]        *[a 7 [[7 [0 1] b] 0 1] c]
    if (instruction == 8 && is_cell(arg)) {
      noun *a = subj, *b = fst(arg), *c = snd(arg); free(arg);
      return nock(L(a, A(7), L(L(A(7), L(A(0), A(1)), b), A(0), A(1)), c));
    }
    // 31 ::    *[a 9 b c]        *[a 7 c 2 [0 1] 0 b]
    if (instruction == 9 && is_cell(arg)) {
      noun *a = subj, *b = fst(arg), *c = snd(arg); free(arg);
      return nock(L(a, A(7), c, A(2), C(A(0), A(1)), A(0), b));
    }
    // 32 ::    *[a 10 [b c] d]   *[a 8 c 7 [0 3] d]
    if (instruction == 10 && is_cell(arg) && is_cell(fst(arg))) {
      noun *a = subj, *b = fst(fst(arg)), *c = snd(fst(arg)), *d = snd(arg);
      free(fst(arg)); free(arg);
      free_noun(b);
      return nock(L(a, A(8), c, A(7), C(A(0), A(3)), d));
    }
    // 33 ::    *[a 10 b c]       *[a c]
    if (instruction == 10 && is_cell(arg)) {
      noun *a = subj, *b = fst(arg), *c = snd(arg); free(arg);
      free_noun(b);
      return nock(C(a, c));
    }
  }
  crash("*a");
}

noun* parse_(const char *str, char **endptr);
noun* parse_num(const char *str, char **endptr) {
  long i = strtol(str, endptr, 10);
  if (*endptr != str) { // we parsed a number
    return A(i);
  } else {
    return NULL;
  }
}
noun* parse_cell(const char *str, char **endptr) {
  if (*str != '[') return NULL;
  str++;
  noun *root, *cell = root = C(NULL, NULL), *prev = NULL;
  while (*str) {
    noun *n = parse_(str, endptr);
    if (!n) { fprintf(stderr, "parse fail: expected a noun at `%.5s'\n", str); exit(1); }
    if (fst(cell) == NULL) fst(cell) = n;
    else { prev = cell; cell = snd(cell) = C(n, NULL); }
    str = *endptr;
    while (isspace(*str)) { str++; (*endptr)++; }
    if (*str == ']') { (*endptr)++; break; }
  }
  if (!*str) { fprintf(stderr, "parse fail: found eos when expecting ]\n"); exit(1); }
  if (!fst(root) || !snd(root)) { fprintf(stderr, "parse fail: not enough things in the cell\n"); exit(1); }
  if (prev) { snd(prev) = fst(cell); free(cell); }
  return root;
}
noun* parse_(const char *str, char **endptr) {
  while (isspace(*str)) str++;
  noun *n = parse_cell(str, endptr);
  if (!n) n = parse_num(str, endptr);
  return n;
}
noun* parse(const char *str) {
  char *endptr = 0;
  return parse_(str, &endptr);
}
