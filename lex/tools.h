#ifndef _TOOLS_H
#define _TOOLS_H
void comment(FILE *fp, char *argv[]);
int ferr(char *fmt, ...);
void print_array(FILE *fp, int *array, int nrows, int ncols);
char *bin_to_ascii(int c, int use_hex);
void fputstr (char *str, int maxlen, FILE *fp);
void yy_init_lex();
int yywrap();
#endif