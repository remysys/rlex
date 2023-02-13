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
void free_nfa();                                          /* egerp.c */
SET *move(SET *inp_set, int c);                           /* egrep.c */
int nfa(char* (*input_function) (void));                  /* egrep.c */

int dfa( char *(*input_function)(void), ROW **dfap,  ACCEPT**	acceptp);          /* dfa.c   */

void pheader(FILE *fp, ROW dtran[], int nrows, ACCEPT *accept); /* print.c */
void pdriver(FILE *output, int nrows, ACCEPT *accept);          /* print.c */

int min_dfa(char *(*input_function)(void), ROW **dfap, ACCEPT **acceptp);  /* minimize.c */
void lerror(int status, char *fmt, ...);        /* lex.c */
int main(int argc, char *argv[]);               /* lex.c */

char *get_expr();                               /*lex.c */

int squash(FILE *fp, ROW *dtran, int nrows, int ncols, char *name);  /* squash.c */
void cnext(FILE *fp, char *name);                                    /* squash.c */

FILE *driver_1(FILE *output, int lines);            /* driver.c */
int driver_2(FILE *output, int lines);              /* driver.c */

#endif