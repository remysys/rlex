#include <stdio.h>

void comment(FILE *fp, char *argv[])
{
  fprintf(fp, "\n/*-----------------------------------------------------\n");

  while (*argv) {
    fprintf(fp, " * %s\n", *argv++);
  }

  fprintf(fp, " */\n\n");
}

void printv (FILE *fp, char *argv[])
{
  /* print an argv-like array of pointers to strings, one string per line
   * the array must be NULL terminated
   */

   while(*argv) {
    fprintf(fp, "%s\n", *argv++);
   }
}