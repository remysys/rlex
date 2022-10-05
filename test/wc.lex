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

int main (int argc, char *argv[]) {
	if (argc != 2) {
		printf("usage: wc $filename\n");
		return 0;
	}

	ii_newfile(argv[1]);
	yylex();
  
  /* remove the first '\n' */
  printf("%-8d%-8d%-8d%-8s\n", lines - 1 , words, chars - 1, argv[1]);
  return 0;
}