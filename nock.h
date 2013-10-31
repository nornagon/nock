#ifndef NOCK_H
#define NOCK_H

#include <stdlib.h>

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

static inline int is_atom(noun* n) {
  return n->as_atom.is_atom & 1; } // if as_cell.b is a pointer, the lowest bit will be 0
static inline int is_cell(noun* n) {
  return (n->as_atom.is_atom & 1) == 0; }

static inline noun* C(noun* a, noun* b) {
  noun* r = malloc(sizeof(noun)); r->as_cell.a = a; r->as_cell.b = b; return r; }
static inline noun* A(atom_t v) {
  noun* r = malloc(sizeof(noun)); r->as_atom.is_atom = 1; r->as_atom.val = v; return r; }

noun* copy_noun(noun* n);

void free_noun(noun* n);

int is_eq(noun* a, noun* b);

void print_noun(noun* n);

noun* parse(const char *str);

noun* nock(noun* n);

#endif  // NOCK_H
