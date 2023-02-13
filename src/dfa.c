#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <compiler.h>
#include <set.h>
#include "globals.h"
#include "dfa.h"

/*----------------------------------------------------------------
 * dfa.c   make a DFA transition table from an NFA created with
 * thompson's construction.
 *
 * Dtran is the deterministic transition table. it is indexed by state number
 * along the major axis and by input character along the minor axis. 
 * Dstates is a list of deterministic states represented as sets of NFA states.
 * Nstates is the number of valid entries in Dtran
 */

typedef struct dfa_state 
{
  unsigned int group;   /* group id, used by minimize() */
  unsigned int mark;    /* mark used by make_dtran() */
  char *accept;         /* accept action if accept state */
  int anchor;           /* anchor point if an accept state */
  SET *set;             /* set of NFA states represented by */
} DFA_STATE;          /* this DFA state */


DFA_STATE *Dstates; /* DFA states table */
ROW *Dtran;         /* DFA transition table */
int Nstates;        /* number of DFA states */
DFA_STATE *Last_marked; /* most-recently marked DFA state in Dtran */


int add_to_dstates(SET *NFA_set, char *accept_string, int anchor) 
{
  int nextstate;

  if (Nstates > (DFA_MAX - 1)) {
    ferr("Too many DFA states\n");
  }

  nextstate = Nstates++;
  Dstates[nextstate].set = NFA_set;
  Dstates[nextstate].accept = accept_string;
  Dstates[nextstate].anchor = anchor;

  return nextstate;
}

int in_dstates(SET *NFA_set) 
{
  /* if there's a set in Dstates that is identical to NFA_set, return the
    * index of the Dstates entry, else return -1.
    */
  DFA_STATE *p;
  DFA_STATE *end = &Dstates[Nstates];

  for (p = Dstates; p < end; p++) {
    if (IS_EQUIVALENT(NFA_set, p->set)) {
      return p - Dstates;
    }
  }
  return -1;
}


DFA_STATE *get_unmarked() 
{
  for (; Last_marked < &Dstates[Nstates]; ++Last_marked) {
    if (!Last_marked->mark) {
      return Last_marked;
    }
  }

  return NULL;
}

void free_sets()
{
  /* free the memory used for the NFA sets in all Dstates entries */
  DFA_STATE *p;
  DFA_STATE *end = &Dstates[Nstates];
  
  for (p = Dstates; p < end; ++p) {
    delset(p->set);
  }
}


void make_dtran(int sstate)
{
  SET *NFA_set;
  DFA_STATE *current;
  int nextstate;
  char *isaccept;
  int anchor;
  int c;


  /* initially Dstates contains a single, unmarked, start state formed by
    * taking the epsilon closure of the NFA start state. So, Dstates[0]
    * (and Dtran[0]) is the DFA start state.
    */
  NFA_set = newset();
  ADD(NFA_set, sstate);

  Nstates = 1;
  Dstates[0].set = e_closure(NFA_set, &Dstates[0].accept, &Dstates[0].anchor);
  Dstates[0].mark = 0;

  while ((current = get_unmarked())) {
    current->mark = 1;
    for (c = MAX_CHARS; --c >= 0; ) {
      if ((NFA_set = move(current->set, c))) {
        NFA_set = e_closure(NFA_set, &isaccept, &anchor);
      }

      if (!NFA_set) {
        nextstate = F;
      } else if ((nextstate = in_dstates(NFA_set)) != -1) {
        delset(NFA_set);
      } else {
        nextstate = add_to_dstates(NFA_set, isaccept, anchor);
      }

      Dtran[current - Dstates][c] = nextstate;
    }
  }

  free_sets();
}


int dfa(char *(*input_function)(void), ROW **dfap, ACCEPT **acceptp) 
{
  /* turns an NFA with the indicated start state (sstate) into a DFA and
    * returns the number of states in the DFA transition table. *dfap is
    * modified to point at that transition table and *acceptp is modified
    * to point at an array of accepting states (indexed by state number).
    * dfa() discards all the memory used for the initial NFA.
    */

  ACCEPT *accept_states;
  int i;

  int start = nfa(input_function);
  Nstates = 0;
  Dstates = (DFA_STATE *) calloc(DFA_MAX, sizeof(DFA_STATE));
  Dtran = (ROW *) calloc(DFA_MAX, sizeof(ROW));

  Last_marked = Dstates;

  if (!Dstates || !Dtran) {
    ferr("can't get memory\n");
  }

  make_dtran(start); /* convert the NFA to a DFA */
  free_nfa();        /* free the memory used for the nfa */

  Dtran = (ROW *) realloc(Dtran, Nstates * sizeof(ROW));
  accept_states = (ACCEPT *) malloc(Nstates * sizeof(ACCEPT));
  
  if (!accept_states || !Dtran) {
    ferr("can't get memory\n");
  }

  for (i = Nstates; --i >= 0; ) {
    accept_states[i].string = Dstates[i].accept;
    accept_states[i].anchor = Dstates[i].anchor;
  }
  
  free(Dstates);
  *dfap = Dtran;
  *acceptp = accept_states;

  if (Verbose) {
    printf("\n%d out of %d DFA states in initial machine.\n", Nstates, DFA_MAX);
    printf("%d bytes required for uncompressed tables.\n\n",
       Nstates * MAX_CHARS * sizeof(TTYPE)   /* dtran  */
       + Nstates * sizeof(TTYPE));           /* accept */

    if (Verbose > 1) {
      printf("The un-minimized DFA looks like this:\n\n");
      pheader(stdout, Dtran, Nstates, accept_states);
    }
  }

  return Nstates;
}
