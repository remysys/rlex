/* fputstr: print a string with control characters mapped to readable strings */
#include <stdio.h>
#include <compiler.h>

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