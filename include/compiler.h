#ifndef _COMPILER_H
#define _COMPILER_H

#define max(a,b) ( ((a) > (b)) ? (a) : (b))
#define min(a,b) ( ((a) < (b)) ? (a) : (b))

#define NUMELE(a) (sizeof(a)/sizeof(*(a)))
#define LASTELE(a) ((a) + (NUMELE(a)-1))
#define TOOHIGH(a, p) ((p) - (a) > (NUMELE(a) - 1))
#define TOOLOW(a, p) ((p) - (a) <  0 )
#define INBOUNDS(a, p) (!(TOOHIGH(a,p) || TOOLOW(a,p)))

/* ---------------- lib/ferr.c ---------------- */
extern int ferr(char *fmt, ...);

/* ---------------- lib/fputstr.c ---------------- */
extern void fputstr (char *str, int maxlen, FILE *fp);

/* ---------------- lib/printv.c ---------------- */
extern void printv(FILE *fp, char *argv[]);
extern void comment(FILE *fp, char *argv[]);

/* ---------------- lib/escape.c ---------------- */
int hex2bin(int c);
int oct2bin(int c);
int esc(char **s);
char *bin_to_ascii(int c, int use_hex);

/* ---------------- lib/printutils.c ---------------- */
void comment(FILE *fp, char *argv[]);
void print_array(FILE *fp, int *array, int nrows, int ncols);
void fputstr (char *str, int maxlen, FILE *fp);

/* ---------------- lib/driver.c ---------------- */
FILE *driver_1(FILE *output, int lines, char *file_name);
int driver_2(FILE *output, int lines);

#endif