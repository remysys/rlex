#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <compiler.h>
#include <set.h>
#include "globals.h"
#include "dfa.h"

/* minimize.c: make a minimal DFA by eliminating equivalent states */

SET *Groups[DFA_MAX];
int Numgroups;      /* number of groups in Groups */
int Ingroup[DFA_MAX];

void pgroups(int nstates) {
  /* print all the groups used for minimization */
  
  SET **current;
  SET **end = &Groups[Numgroups];
  for (current = Groups; current < end; ++current) {
    printf( "\tgroup %ld: {", (long)(current - Groups));
    pset(*current, (pset_t)fprintf, (void*)stdout);
    printf( "}\n");
  }
  printf("\n");
  while (--nstates >= 0) {
    printf("\tstate %2d is in group %2d\n", nstates, Ingroup[nstates]);
  }
}

void init_groups(int nstates, ACCEPT *accept)
{
  SET **last = &Groups[0];
  Numgroups = 0;
  int i, j;

  for (i = 0; i < nstates; i++) {
    for (j = i; --j >= 0; ) {
      if (accept[i].string == accept[j].string) {
        ADD(Groups[Ingroup[j]], i);
        Ingroup[i] = Ingroup[j];
        goto match;
      }
    }
    *last = newset();
    ADD(*last, i);
    Ingroup[i] = Numgroups++;
    ++last;
match:;    /* group already exists, keep going */
  }

  if (Verbose > 1) {
    printf("initial groupings:\n");
    pgroups(nstates);
  }
}

void fix_dtran(ROW **dfap, ACCEPT **acceptp) 
{
  /* reduce the size of the dtran, using the group set made by minimize().
    * return the state number of the start state. The original dtran and accept
    * arrays are destroyed and replaced with the smaller versions.
    * consider the first element of each group (current) to be a
    * "representative" state. copy that state to the new transition table,
    * modifying all the transitions in the representative state so that they'll
    * go to the group within which the old state is found.
    */

  ROW *newdtran;
  ACCEPT *newaccept;
  SET **current;
  SET **end = &Groups[Numgroups];
  int *dest;
  int *src;
  ROW *dtran = *dfap;
  ACCEPT *accept = *acceptp;
  int state;
  int i;

  newdtran = (ROW *) calloc(Numgroups, sizeof(ROW));
  newaccept = (ACCEPT *) calloc(Numgroups, sizeof(ACCEPT));

  if (!newdtran || !newaccept) {
    ferr("can't get memory\n");
  }

  next_member(NULL);
  for (current = Groups; current < end; ++current) {
    dest = &newdtran[current - Groups][0];
    state = next_member(*current);
    src = &dtran[state][0];
    newaccept[current - Groups] = accept[state];
    for (i = MAX_CHARS; --i >= 0; src++, dest++) {
      *dest = (*src == F) ? F : Ingroup[*src];
    }
  }

  free(*dfap);
  free(*acceptp);

  *dfap = newdtran;
  *acceptp = newaccept;
}

void minimize(int nstates, ROW **dfap, ACCEPT **acceptp)
{
  int old_numgroups;
  int c;
  SET **current;
  SET **new;
  
  int first;
  int next;
  int goto_first;
  int goto_next;

  ROW *dtran = *dfap;
  ACCEPT *accept = *acceptp;
  init_groups(nstates, accept);

  do {
    old_numgroups = Numgroups;
    for (current = &Groups[0]; current < &Groups[Numgroups]; ++current) {
      if (num_ele(*current) <= 1) {
        continue;
      }
      new = &Groups[Numgroups];
      *new = newset();
      next_member(NULL);
      first = next_member(*current);
      
      while ((next = next_member(*current)) >= 0) {
        for (c = MAX_CHARS; --c >= 0; ) {
          goto_first = dtran[first][c];
          goto_next  = dtran[next][c];

          if ((goto_next != goto_first) && (goto_first == F || goto_next == F || Ingroup[goto_first] != Ingroup[goto_next])) {
            REMOVE(*current, next);
            ADD(*new, next);
            Ingroup[next] = Numgroups;
            break;
          }
        }
      }

      if (IS_EMPTY(*new)) {
        delset(*new);
      } else {
        ++Numgroups;
      }
    }
  } while (old_numgroups != Numgroups);

  fix_dtran(dfap, acceptp);

  if (Verbose > 1) {
    printf("\nstates grouped as follows after minimization:\n"); 
    pgroups( nstates );
  }
}

int min_dfa(char *(*input_function)(void), ROW **dfap, ACCEPT **acceptp) 
{
  /* make a minimal DFA, eliminating equivalent states. Return the number of
    * states in the minimized machine. *sstatep = the new start state.
    */
  int nstates; /* number of DFA states */
  memset(Groups, 0, sizeof(Groups));
  memset(Ingroup, 0, sizeof(Ingroup));
  Numgroups = 0;
  nstates = dfa(input_function, dfap, acceptp);
  minimize(nstates, dfap, acceptp);

  return Numgroups;
}

#ifdef MAIN
char *getstr()
{
  static char bufs[80];
  printf("%d: ", Lineno++);
  if (fgets(bufs, NUMELE(bufs), stdin)) {
      return bufs;
  } else {
      return NULL;
  }
}

int main(int argc, char *argv[])
{
  int	   nstates;		
  ROW	   *dtran;		
  ACCEPT *accept;		
  int	   i;
  Verbose = 2;

  nstates = min_dfa(getstr, &dtran, &accept );
  printf("nstates: %d\n", nstates);
}
#endif