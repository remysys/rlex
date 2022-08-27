#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <compiler.h>

void comment(FILE *fp, char *argv[])
{
  fprintf(fp, "\n/*-----------------------------------------------------\n");

  while (*argv) {
    fprintf(fp, " * %s\n", *argv++);
  }

  fprintf(fp, " */\n\n");
}

/* general-purpose subroutine to print out a 2-dimensional array */

#define NCOLS	 10   	/* number of columns used to print arrays	*/

void print_array(FILE *fp, int *array, int nrows, int ncols)
{
  /* array: dfa transition table 
   * nrows: number of rows in array
   * ncols: number of columns in array
   * print the C source code to initialize the two-dimensional array pointed
   * to by "array." print only the initialization part of the declaration.
   */

  int r;
  int c; 
  fprintf(fp, "{\n");

  for (r = 0; r < nrows; r++) {
    fprintf(fp, "/* %02d */ { ", r);
    for (c = 0; c < ncols; c++) {
      fprintf(fp, "%3d", *array++);
      if (c < ncols - 1) {
        fprintf(fp, ", ");
      }

      if (c % NCOLS == NCOLS - 1 && c != ncols - 1) {
        fprintf(fp, "\n            ");
      }
    }

    if (c > NCOLS) {
      fprintf(fp, "\n            ");
    }

    fprintf(fp, " }%c\n", r < nrows - 1 ? ',' : ' ');
  }
  fprintf(fp, "};\n");
}


/* fputstr: print a string with control characters mapped to readable strings */

void fputstr (char *str, int maxlen, FILE *fp)
{
  char *s;
  while (*str && maxlen >= 0) {
    s = bin_to_ascii(*str++, 1);
    while (*s && --maxlen >= 0) {
      putc(*s++, fp);
    }
  }
}