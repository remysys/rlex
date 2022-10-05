#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <compiler.h>
#include "globals.h"
#include "nfa.h"
#include "dfa.h"

/* print.c:  this module contains miscellaneous print routines */

static FILE *Input_file = NULL;   /* lex.par default */
static int Input_line;              /* line number of most-recently read line */
static char *File_name = "/usr/local/lib64/lex.par"; /* template-file name, where the lex.par be installed */

FILE *driver_1(FILE *output, int lines) 
{
	
  if (!(Input_file = fopen("lex.par", "r"))) {
    if (!(Input_file = fopen(File_name, "r"))) {
      return NULL;
    }
  }

  Input_line = 0;
  driver_2(output, lines);
  return Input_file;
}

int driver_2(FILE *output, int lines)
{
  static char buf[256];
  char *p;
  int processing_comment = 0;
  if (!Input_file) {
    ferr("internal error [driver_2], template file lex.par not open\n");
  }

  if (lines) {
    fprintf(output, "\n#line %d \"%s\"\n", Input_line + 1, File_name);
  }

  while(fgets(buf, sizeof(buf), Input_file)) {
    ++Input_line;
    if (*buf == '?') {
      break;
    }

    for (p = buf; isspace(*p); ++p) {
      ;
    }

    if (*p == '@') {
      processing_comment = 1;
      continue;
    } else if (processing_comment) {  /* previous line was a comment */
      /* but current line is not */
      processing_comment = 0;
      if (lines) {
        fprintf(output, "\n#line %d \"%s\"\n", Input_line, File_name);
      }
    }

    fputs(buf, output);
  }

  return feof(Input_file);
}


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

  fprintf(fp, "#ifdef __NEVER__\n");
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



void pdriver(FILE *output, int nrows, ACCEPT *accept)
{
  /* nrows: number of states in dtran[] 
   * accept: set of accept states in dtran[] 
   * print the array of accepting states, the driver itself, and the case
   * statements for the accepting strings
   */

  int i;
  static  char  *text[] = {
    "the Yyaccept array has two purposes. If Yyaccept[i] is 0 then state",
    "i is nonaccepting. If it's nonzero then the number determines whether",
    "the string is anchored, 1=anchored at start of line, 2=at end of",
    "line, 3=both, 4=line not anchored",
    NULL
  };

  comment(output, text);
  fprintf(output, "YYPRIVATE YY_TTYPE Yyaccept[] =\n");
  fprintf(output, "{\n");

  for (i = 0; i < nrows; i++) { /*accepting array */
    if (!accept[i].string) {
      fprintf(output, "\t0  ");
    } else {
      fprintf(output, "\t%-3d",accept[i].anchor ? accept[i].anchor : 4);
    }
    fprintf(output, "%c    /* state %-3d */\n", i == (nrows -1) ? ' ' : ',' , i);
  }

  fprintf(output, "};\n\n");

  driver_2(output, !No_lines); /* code above cases */

  for (i = 0; i < nrows; i++) { /* case statements */
    if (accept[i].string) {
      fprintf(output, "\t\t\t\t\tcase %d:\t\t\t\t\t/* state %-3d */\n",i, i);
      if (!No_lines) {
        fprintf(output, "#line %d \"%s\"\n", *((int *)(accept[i].string) - 1), Input_file_name);
      }

      fprintf(output, "\t\t\t\t\t\t  %s\n", accept[i].string);
      fprintf(output, "\t\t\t\t\t\t  break;\n");
    }
  }

  driver_2(output, !No_lines); /* code below cases */
}


