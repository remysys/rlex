#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <compiler.h>

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