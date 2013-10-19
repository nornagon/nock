#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
typedef unsigned long atom_t;
typedef struct noun {
  union {
    struct { struct noun *a, *b; } as_cell;
    struct { atom_t val; unsigned long is_atom; } as_atom;
  };
} noun;
#define fst(n) n->as_cell.a
#define snd(n) n->as_cell.b
#define val(n) n->as_atom.val
int is_atom(noun* n) { return n->as_atom.is_atom & 1; } // if as_cell.b is a pointer, the lowest bit will be 0
int is_cell(noun* n) { return (n->as_atom.is_atom & 1) == 0; }
noun* C(noun* a, noun* b) { noun* r = malloc(sizeof(noun)); r->as_cell.a = a; r->as_cell.b = b; return r; }
noun* A(atom_t v) { noun* r = malloc(sizeof(noun)); r->as_atom.is_atom = 1; r->as_atom.val = v; return r; }
void free_noun(noun* n) { if (n && is_cell(n)) { free_noun(n->as_cell.a); free_noun(n->as_cell.b); } free(n); }
noun* copy_noun(noun* n) {
  if (is_cell(n)) return C(copy_noun(fst(n)), copy_noun(snd(n)));
  else return A(val(n));
}

void print_noun(noun* n) {
  if (is_atom(n)) printf("%lu", val(n));
  else { printf("["); print_noun(fst(n)); printf(" "); print_noun(snd(n)); printf("]"); }
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
  printf("*"); print_noun(n); printf("\n");
  if (is_atom(n)) return n;
  noun* formula = snd(n);
  noun* subj = fst(n);
  free(n);
  // 19 ::    *[a [b c] d]      [*[a b c] *[a d]]
  if (is_cell(fst(formula))) {
    noun *a = subj, *b = fst(fst(formula)), *c = snd(fst(formula)), *d = snd(formula);
    noun *a2 = copy_noun(a);
    free(formula);
    return C(nock(C(a, C(b, c))), nock(C(a2, d)));
  }
  // 21 ::    *[a 0 b]          /[b a]
  if (is_atom(fst(formula)) && val(fst(formula)) == 0) {
    noun *a = subj, *b = snd(formula);
    free(fst(formula)); free(formula);
    return slot(C(b, a));
  }
  // 22 ::    *[a 1 b]          b
  if (is_atom(fst(formula)) && val(fst(formula)) == 1) {
    noun *a = subj, *b = snd(formula);
    free(fst(formula)); free(formula); free(a);
    return b;
  }
  // 23 ::    *[a 2 b c]        *[*[a b] *[a c]]
  if (is_atom(fst(formula)) && val(fst(formula)) == 2 && is_cell(snd(formula))) {
    noun *a = subj, *b = fst(snd(formula)), *c = snd(snd(formula));
    noun *a2 = copy_noun(a);
    free(fst(formula)); free(snd(formula)); free(formula);
    return nock(C(nock(C(a, b)), nock(C(a2, c))));
  }
  // 24 ::    *[a 3 b]          ?*[a b]
  if (is_atom(fst(formula)) && val(fst(formula)) == 3) {
    noun *a = subj, *b = snd(formula);
    free(fst(formula)); free(formula);
    noun* tmp = nock(C(a, b));
    noun* r = is_cell(tmp) ? A(0) : A(1);
    free_noun(tmp);
    return r;
  }
  // 25 ::    *[a 4 b]          +*[a b]
  if (is_atom(fst(formula)) && val(fst(formula)) == 4) {
    noun *a = subj, *b = snd(formula);
    free(fst(formula)); free(formula);
    return inc(nock(C(a,b)));
  }
  // 26 ::    *[a 5 b]          =*[a b]
  if (is_atom(fst(formula)) && val(fst(formula)) == 5) {
    noun *a = subj, *b = snd(formula);
    free(fst(formula)); free(formula);
    return eq(nock(C(a,b)));
  }
  exit(1); // TODO better crash
}

noun* parse(const char *str, char **endptr);
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
    noun *n = parse(str, endptr);
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
noun* parse(const char *str, char **endptr) {
  while (isspace(*str)) str++;
  noun *n = parse_cell(str, endptr);
  if (!n) n = parse_num(str, endptr);
  return n;
}

int main(int argc, char *argv[]) {
  char *endptr = 0;
  noun *n = parse(argv[1], &endptr);
  printf("> "); print_noun(n); printf("\n");
  noun *res = nock(n);
  print_noun(res); printf("\n");
  free_noun(res);
  return 0;
}
