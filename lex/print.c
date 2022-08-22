#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "set.h"
#include "nfa.h"
#include "dfa.h"
#include "globals.h"
#include "tools.h"

/* print.c:  this module contains miscellaneous print routines that do
 * everything except print the actual tables.
 */

void pheader(FILE *fp, ROW dtran[], int nrows, ACCEPT *accept)
{
  /* print out a header comment that describes the uncompressed DFA 
   * fp: output stream
   * dtran: dfa transition table
   * number of states in dtran
   * set of accept states in dtran
   */
  int i, j;
  int last_transition;
  int chars_printed;

  fprintf(fp, "ifdef __NEVER__\n");
  fprintf(fp, "/*---------------------------------------------------\n");
  fprintf(fp, " * DFA (start state is 0) is:\n *\n");

  for (i = 0; i < nrows; i++) {
    if (!accept[i].string) {
      fprintf(fp, " * state %d [nonaccepting]", i);
    } else {
      fprintf(fp, " * State %d [accepting, line %d <", i, ((int *)(accept[i].string))[-1]);
      fputstr(accept[i].string, 20, fp);
	    fprintf(fp, ">]");
      if (accept[i].anchor) {
        fprintf(fp, "Anchor: %s%s", accept[i].anchor & START ? "start" : "",
                                    accept[i].anchor & END ? "end" : "");
      }
    }

    last_transition = -1;
    for (j = 0; j < MAX_CHARS; j++) {
      if (dtran[i][j] != F) {
        if (dtran[i][j] != last_transition) {
          fprintf(fp, "\n *    goto %2d on ", dtran[i][j]);
          chars_printed = 0;
        }

        fprintf(fp, "%s", bin_to_ascii(j, 1));

        if ((chars_printed += strlen(bin_to_ascii(j, 1))) > 56) {
          fprintf(fp, "\n *               ");
          chars_printed = 0;
        }

        last_transition = dtran[i][j];
      }
    }

    fprintf(fp, "\n");
  }

  fprintf(fp, " */\n\n");
  fprintf(fp, "#endif\n");
}


