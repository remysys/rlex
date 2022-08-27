/* wordcount just like unix wc */
%{
#include <string.h>

int chars = 0;
int words = 0;
int lines = 0;
%}

%%

[a-zA-Z]+	{ words++; chars += strlen(yytext); }
\n		    { chars++; lines++; }
.		      { chars++; }
%%

int main() {
  yylex();
  
  /* remove the first '\n' */
  printf("%-3d%-3d%-3d\n", lines - 1 , words, chars - 1);
  return 0;
}