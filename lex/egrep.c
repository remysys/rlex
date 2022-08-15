#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "comm.h"
#include "set.h"
#include "nfa.h"

#define MAX_INT (int)(((unsigned)(~0)) >> 1)

NFA *Nfa; /* base address of NFA array */
int Num_states; /* number of states in NFA */

int nfa(char* (*input_function) (void)) 
{
  /* compile the NFA and initialize the various global variables used by
     * move() and e_closure(). return the state number (index) of the NFA start
     * state. This routine must be called before either e_closure() or move()
     * are called. The memory used for the nfa can be freed with free_nfa()
     * (in thompson.c).
     */

     NFA *sstate;
     Nfa = thompson(input_function, &Num_states, &sstate);

     return (sstate - Nfa);
}

void free_nfa()
{
  free(Nfa);
}

SET *e_closure(SET *input, char **accept, int *anchor)
{
  /* input is the set of start states to examine.
     * *accept  is modified to point at the string associated with an accepting
     *	        state (or to NULL if the state isn't an accepting state).
     * *anchor  is modified to hold the anchor point, if any.
     *
     * computes the epsilon closure set for the input states. The output set
     * will contain all states that can be reached by making epsilon transitions
     * from all NFA states in the input set. returns an empty set if the input
     * set or the closure set is empty, modifies *accept to point at the
     * accepting string if one of the elements of the output state is an
     * accepting state.
     */

    int stack[NFA_MAX]; 
    int *sp;
    int i;
    int accept_num;
    NFA *p;
    
    if (!input) {
      goto abort;
    }

    *accept = NULL;
    accept_num = MAX_INT;

    sp = &stack[-1];
    for(next_member(NULL); (i = next_member(input)) >= 0; ) {
      *++sp = i;
    }

    while(INBOUNDS(stack, sp)) {
      i = *sp--;
      p = &Nfa[i];
      if (p->accept && i < accept_num) {
        accept_num = i;
        *accept = p->accept;
        *anchor = p->anchor;
      }

      if (p->edge == EPSILON) {
        if (p->next) {
          i = p->next - Nfa;
          if (!MEMBER(input, i)) {
            ADD(input, i);
            *++sp = i;
          }
        }

        if (p->next2) {
          i = p->next2 - Nfa;
          if (!MEMBER(input, i)) {
            ADD(input, i);
            *++sp = i;
          }
        }
      }
    }

abort:
    return input;
}

SET *move(SET *inp_set, int c)
{
  /* return a set that contains all NFA states that can be reached by making
    * transitions on "c" from any NFA state in "inp_set." returns NULL if
    * there are no such transitions. the inp_set is not modified.
    */
    
  int i;
  NFA *p;
  SET *outset = NULL;

  for (i = Num_states; --i >= 0; ) {
    if (MEMBER(inp_set, i)) {
      p = &Nfa[i];
      if (p->edge == c || (p->edge == CCL && TEST(p->bitset, c))) {
        if (!outset) {
          outset = newset();
          ADD(outset, p->next - Nfa);
        }
      }
    }
  }

  return outset;
}

#ifdef MAIN
#include "globals.h"

#define BSIZE 256

char Buf[BSIZE];
char *Pbuf = Buf;
char *Expr;

int nextchar (void) 
{
  if (!*Pbuf) {
    if (!fgets(Buf, BSIZE, stdin)) {
      return 0;
    }
    Pbuf = Buf;
  }
  return *Pbuf++;
}

void printbuf(void) 
{
  fputs(Buf, stdout); /* print the buffer and force a read */
  *Pbuf = 0;          /* on the next call to nextchar() */
}

char *get_regex (void) 
{
  static int first_time = 1;
  if (!first_time) {
    return NULL;
  }
  first_time = 0;
  return Expr;
}

int main(int argc, char *argv[]) 
{
  if (argc != 2) {
    fprintf(stderr, "usage: egrep pattern <input");
    exit(1);
  }
  int sstate;
  SET *start_dfastate;
  SET *current;
  SET *next;
  char *accept;
  int anchor;
  int c;

   Verbose = 2;

  /*  1: compile the NFA; initialize move() & e_closure().
      *  2: create the initial state, the set of all NFA states that can
      *	   be reached by making epsilon transitions from the NFA start state.
      *	   note that e_closure() returns the original set with elements
      *	   added to it as necessary.
      *  3: initialize the current state to the start state.
      */
  
  Expr = argv[1];
  sstate = nfa(get_regex);
  
  start_dfastate = newset();
  ADD(start_dfastate, sstate);
  
  if (!e_closure(start_dfastate, &accept, &anchor)) {
    fprintf(stderr, "internal error: state machine is empty\n");
    exit(1);
  }

  current = newset();
  ASSIGN(current, start_dfastate);

  /* now interpret the NFA: the next state is the set of all NFA states that
    * can be reached after we've made a transition on the current input
    * character from any of the NFA states in the current state. the current
    * input line is printed every time an accept state is encountered.
    * the machine is reset to the initial state when a failure transition is
    * encountered.
    */
    
  while((c = nextchar())) {
    if (next = e_closure(move(current, c), &accept, &anchor)) {
      if (accept) {
        printbuf();
      } else {
        delset(current);
        current = next;
        continue;
      }
    }
    if (next) {
      delset(next);
    }
    ASSIGN(current, start_dfastate);
  }

  delset(current);
  delset(start_dfastate);
  return 0;
}

#endif 
