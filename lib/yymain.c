/* a default main module to test the lexical analyzer */
#include <stdio.h>
#include <l.h>

int main(int argc, char *argv[])
{
  
  int yylex(void);

  if (argc == 2) {
    ii_newfile(argv[1]);
  }

  while(yylex());
  
  return 0;
}