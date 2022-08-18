#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "tools.h"

void comment(FILE *fp, char *argv[])
{
  fprintf(fp, "\n/*-----------------------------------------------------\n");

  while (*argv) {
    fprintf(fp, " * %s\n", *argv++);
  }

  fprintf(fp, " */\n\n");
}

int ferr(char *fmt, ...) 
{
  va_list args;

  va_start(args, fmt);

  if (fmt) {
    fprintf(stderr, fmt, args);
  } else {
    perror(va_arg(args, char *));
  }

  exit(errno);
  return 0;
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

char *bin_to_ascii(int c, int use_hex)
{
  /* return a pointer to a string that represents c. this will be the
    * character itself for normal characters and an escape sequence (\n, \t,
    * \x00, etc., for most others). A ' is represented as \'. the string will
    * be destroyed the next time bin_to_ascii() is called. ff "use_hex" is true
    * then \xDD escape sequences are used. otherwise, octal sequences (\DDD)
    * are used
    */

  static char buf[8];
  c  &= 0xff;
  if (' ' <= c && c < 0x7f && c != '\'' && c != '\\') {
    buf[0] = c;
    buf[1] = '\0';
  } else {
    buf[0] = '\\';
    buf[2] = '\0';
    
    switch (c) {
      case '\\': buf[1] = '\\'; break;
      case '\'': buf[1] = '\''; break;
      case '\b': buf[1] = 'b' ; break;
      case '\f': buf[1] = 'f' ; break;
      case '\t': buf[1] = 't' ; break;
      case '\r': buf[1] = 'r' ; break;
      case '\n': buf[1] = 'n' ; break; 
      default: sprintf(&buf[1], use_hex? "x%03x" : "%03o", c); break;
    }
  }

  return buf;
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