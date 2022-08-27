#ifndef _NFA_H
#define _NFA_H

typedef struct NFA 
{
  int edge;
  SET *bitset;
  struct NFA *next;
  struct NFA *next2;
  char *accept;
  int anchor;
} NFA;

#define EPSILON -1
#define CCL     -2
#define EMPTY   -3

/* values of the anchor field */
#define NONE    0
#define START   1
#define END     2
#define BOTH (START | END);

#define NFA_MAX 1024
#define STR_MAX (100 * 1024)

void print_nfa(NFA *, int, NFA *);
void new_macro(char *def);
void printmacs();
NFA *thompson(char *(*input_func)(), int *max_state, NFA **start_state);
#endif
