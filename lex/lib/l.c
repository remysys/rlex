#include <stdio.h>
#include <stdlib.h>
#include "l.h"

void yy_init_lex()
{
  /* default initialization routine--does nothing */
}

int yywrap() /* yylex() halts if 1 is returned */
{
  return 1;
}


/* a default main module to test the lexical analyzer */
/* 
int main(int argc, char *argv[])
{
  if (argc == 2) {
    ii_newfile(argv[1]);
  }

  while(yylex());
  
  return 0;
}
*/

