#ifndef _DFA_H
/*@A (C) 1992 Allen I. Holub                                                */
/*--------------------------------------------------------------
 * DFA.H: The following definitions are used in dfa.c and in
 * minimize.c to represent DFA's.
 *--------------------------------------------------------------
 */

#ifndef __DFA_H
#define __DFA_H

#define DFA_MAX 254 
#define MAX_CHARS 128

typedef unsigned char TTYPE;
#define F -1


typedef int ROW[MAX_CHARS];

typedef struct ACCEPT
{
  char *string; /* accepting string; null if nonaccepting */
  int anchor;
} ACCEPT;

SET *e_closure(SET *input, char **accept, int *anchor);   /* egerp.c */
void free_nfa()                                           /* egerp.c */
SET *move(SET *inp_set, int c)                            /* egrep.c */
int nfa(char* (*input_function) (void));                  /* egrep.c */

int dfa( char *(*input_function)(void), ROW **dfap,  ACCEPT**	acceptp);          /* dfa.c   */

#endif