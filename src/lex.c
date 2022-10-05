#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <compiler.h>
#include "globals.h"
#include "dfa.h"

#define DTRAN_NAME "Yy_nxt" /* name used for DFA transition table. up to 
				                     * 3 characters are appended to the end of
				                     * this name in the row-compressed tables.   */

#define E(x)  fprintf(stderr, "%s\n", x);

int  Verbose	        = 0 ;	      /* print statistics		   */
int  No_lines         = 1 ;	      /* suppress #line directives	   */
int  Public	          = 0 ;	      /* make static symbols public    */
int  Actual_lineno    = 1 ;	      /* current input line number	   */
int  Lineno	          = 1 ;	      /* line number of first line of  a multiple-line rule.	   */
char Input_buf[MAXINP];		        /* line buffer for input	   */
char *Input_file_name;		        /* input file name (for #line   */
FILE *Ifile;			                /* input stream.		   */
FILE *Ofile;			                /* output stream.		   */

static int Column_compress = 1;   /* variables for command-line switches */
static int No_compression  = 0;
static int No_header       = 0;
static int Header_only     = 0;

#define VERSION "0.01 [gcc 4.8.5]"

void lerror(int status, char *fmt, ...)
{
  /* print an error message and input line number. exit with
   * indicated status if "status" is nonzero.
   */
  
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "rlex, input line %d: ", Actual_lineno);
  vfprintf(stderr, fmt, args);
  va_end(args);

  if (status) {
    exit(status);
  }
}

void strip_comments(char *string)
{
  /* scan through the string, replacing c-like comments with space
   * characters. multiple-line comments are supported
   */
  
  static int incomment = 0;
  for (; *string; ++string) {
    if (incomment) {
      if (string[0] == '*' && string[1] == '/') {
        incomment = 0;
        *string++ = ' ';
        *string = ' ';
        continue;
      }

      if (!isspace(*string)) {
        *string = ' ';
      }

    } else {
      if (string[0] == '/' && string[1] == '*') {
        incomment = 1;
        *string++ = ' ';
        *string = ' ';
      }
    }
  }
}

static int getrule(char **stringp, int n, FILE *stream)
{
  /* gets a line of input. gets at most n-1 characters. updates *stringp
   * to point at the '\0' at the end of the string. return a lookahead
   * character (the character that follows the \n in the input). the '\n'
   * is not put into the string.
   *
   * return the character following the \n normally,
   *   EOF  at end of file,
   *	 0 if the line is too long.
   */
  
  static int lookahead = 0;
  char *str = *stringp;

  if (lookahead == 0) {
    lookahead = getc(stream);
  }

  if (n > 0 && lookahead != EOF) {
    while(--n > 0) {
      *str = lookahead;
      lookahead = getc(stream);
      if (*str == '\n' || *str == EOF) {
        break;
      }
      str++;
    }
    *str = '\0';
    *stringp = str;
  }

  return (n <= 0) ? 0 : lookahead;
}

char *get_expr()
{
  /* input routine for nfa(). gets a regular expression and the associated
   * string from the input stream. returns a pointer to the input string
   * normally. returns NULL on end of file or if a line beginning with % is
   * encountered. all blank lines are discarded and all lines that start with
   * whitespace are concatenated to the previous line. the global variable
   * Lineno is set to the line number of the top line of a multiple-line
   * block. Actual_lineno holds the real line number.
   */

  static int lookahead = 0;
  int space_left = MAXINP;
  char *p = Input_buf;

  if (Verbose > 1) {
    printf("b%d: ", Actual_lineno);
  }
  
  if (lookahead == '%') {
    return NULL; /* next line starts with a % sign, return End-of-input marker */
  }

  Lineno = Actual_lineno;

  while((lookahead = getrule(&p, space_left, Ifile)) != EOF) {
    if (!lookahead) {
      lerror(1, "rule too long\n");
    }

    
    Actual_lineno++;
    if (!Input_buf[0]) {
      continue; /* ignore blank lines */
    }

    space_left = MAXINP - (p - Input_buf);

    if (!isspace(lookahead)) {
      break;
    }
    *p++ = '\n';
    space_left = space_left - 1;
  }

  if (Verbose > 1) {
    printf("%s\n", lookahead ? Input_buf : "--EOF--");
  }

  return lookahead ? Input_buf : NULL;
}

void signon()
{
  /* print the sign-on message. since the console is opened explicitly, the
   * message is printed even if both stdout and stderr are redirected.
   */
  FILE *screen;
  if (!(screen = fopen("/dev/tty", "w"))) {
    screen = stderr;
  }

  fprintf(screen, "rlex %s [%s]. (c) %s, ****.", VERSION, __DATE__,  __DATE__ + 7);
  fprintf(screen," all rights reserved.\n");

  if (screen != stderr) {
    fclose(screen);
  }
}

void cmd_line_error(int usage, char *fmt, ...)
{
  /* print an error message and exit to the operating system. this routine is
   * used much like printf(), except that it has an extra first argument.
   * if "usage" is 0, an error message associated with the current value of
   * errno is printed after printing the format/arg string in the normal way.
   * if "usage" is nonzero, a list of legal command-line switches is printed.
   */

  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "rlex: ");
  vfprintf(stderr, fmt, args);
  if (!usage) {
    perror("");
  } else {
    E("\n\nusage is: rlex [options] file");
	  E("-f  for (f)ast. don't compress tables");
 	  E("-h  suppress (h)eader comment that describes state machine");
	  E("-H  print the (H)eader only");
	  E("-l  suppress #(l)ine directives in the output");
	  E("-t  send output to standard output instead of yylex.c");
	  E("-v  (v)erbose mode, print statistics");
	  E("-V  more (V)erbose, print internal diagnostics as rlex runs");
  }
  
  va_end(args);
  exit(1);
}


/* head processes everything up to the first %%. any lines that begin
 * with white space or are surrounded by %{ and %} are passed to the
 * output. all other lines are assumed to be macro definitions.
 * a %% can not be concealed in a %{ %} but it must be anchored at start
 * of line so a %% in a printf statement (for example) is passed to the
 * output correctly. similarly, a %{ and %} must be the first two characters
 * on the line.
 */

void head(int head_only) 
{
  int in_codeblock = 0; /* true if in a %{ %} block */
  if (!head_only && Public) {
    fputs("#define YYPRIVATE\n\n", Ofile);
  }

  if (!No_lines) {
    fprintf( Ofile, "#line 1 \"%s\"\n", Input_file_name);
  }

  while (fgets(Input_buf, MAXINP, Ifile)) {
    ++Actual_lineno;
    if (!in_codeblock) { /* Don't strip comments from code blocks */
      strip_comments(Input_buf);
    }

    if (Verbose > 1) {
      printf("h%d: %s", Actual_lineno, Input_buf);
    }

    if (Input_buf[0] == '%') {
      if (Input_buf[1] == '%') {
        if (!head_only) {
          fputs("\n", Ofile);
        }
        break;
      } else {
        if (Input_buf[1] == '{') {
          in_codeblock = 1;
        } else if (Input_buf[1] == '}') {
          in_codeblock = 0;
        } else {
          lerror(0, "ignoring illegal %%%c directive\n", Input_buf[1]);
        }
      }
    } else if (in_codeblock || isspace(Input_buf[0])) {
      if (!head_only) {
        fputs(Input_buf, Ofile);
      }
    } else {
      new_macro(Input_buf);
      if (!head_only) {
        fputs("\n", Ofile); /* replace macro def with a blank 
                             * line so that the line numbers 
                             * won't get messed up.
                             */
      }
    }
  }

  if (Verbose > 1) {
    printmacs();
  }
}

void tail()
{
  fgets(Input_buf, MAXINP, Ifile); /* throw away the line that had the %% on it */

  if (!No_lines) {
    fprintf(Ofile, "#line %d \"%s\"\n", Actual_lineno + 1, Input_file_name);
  }

  while (fgets(Input_buf, MAXINP, Ifile)) {
    if (Verbose > 1) {
      printf("t%d: %s", Actual_lineno++, Input_buf);
    }

    fputs(Input_buf, Ofile);
  }
}


void defnext(FILE *fp, char *name)
{
  /* print the default yy_next(s,c) subroutine for an uncompressed table */
  static char *comment_text[] = 
  {
    "yy_next(state,c) is given the current state and input character and",
    "evaluates to the next state",
    NULL
  };

  comment(fp, comment_text);
  fprintf(fp, "#define yy_next(state, c)  (%s[state][c])\n", name);
}

void do_file()
{
  int nstates;    /* number of DFA states */
  ROW *dtran;     /* transition table */
  ACCEPT *accept; /* set of accept states in dfa */
  int i;

  /* process the input file */

  head(Header_only);  /* print everything up to first %% */

  nstates = min_dfa(get_expr, &dtran, &accept); /* make dfa */
  
  if (Verbose) {
    printf("%d out of %d DFA states in minimized machine\n", nstates, DFA_MAX);
	  printf("%d bytes required for minimized tables\n\n",
		  nstates * MAX_CHARS * sizeof(TTYPE)		/* dtran  */
	       + nstates * sizeof(TTYPE));		    /* accept */
  }

  if (!No_header) {
    pheader(Ofile, dtran, nstates, accept);  /* print header comment*/
  }

  if (!Header_only) {
    /* first part of driver */
    if (!driver_1(Ofile, !No_lines)) {
      perror("lex.par");
      exit(1);
    }

    if (!No_compression) {
      fprintf(Ofile, "YYPRIVATE YY_TTYPE %s[%d][%d] = \n", 
        DTRAN_NAME, nstates, MAX_CHARS);
      print_array(Ofile, (int *)dtran, nstates, MAX_CHARS);
      defnext(Ofile, DTRAN_NAME);
    } else if (Column_compress) {
      i = squash(Ofile, dtran, nstates, MAX_CHARS, DTRAN_NAME);
      cnext(Ofile, DTRAN_NAME);
      if (Verbose) {
        printf("%d bytes required for column-compressed tables\n\n",
		      i /* dtran      */
		      + (nstates * sizeof(int))); /* Yyaccept  */
      }
    }

    pdriver(Ofile, nstates, accept); /* print rest of driver and */
    tail();                          /* everything following the second %% */
  }
}

int main(int argc, char *argv[])
{
  static char *p;
  static int use_stdout = 0;
  signon();

  for (++argv, --argc; argc && *(p = *argv) == '-'; ++argv, --argc) {
    while (*++p) {
      switch (*p) {
        case 'f': No_compression = 1; break;
        case 'h': No_header = 1; break;
        case 'H': Header_only = 1; break;
        case 'l': No_lines = 1; break;
        case 'p': Public = 1; break;
        case 't': use_stdout = 1; break;
        case 'v': Verbose = 1; break;
        case 'V': Verbose = 2; break;
        default: cmd_line_error(1, "-%c illegal argument", *p);
          break;
      }
    }
  }

  if (argc > 1) {
    cmd_line_error(1, "too many argments. only one file name permitted");
  } else if (argc <= 0) {
    cmd_line_error(1, "file name required");
  } else { /* argc == 1 */
    if (Ifile = fopen(*argv, "r")) {
      Input_file_name = *argv;
    } else {
      cmd_line_error(0, "can't open input file %s", *argv);
    }
  }

  if (!use_stdout) {
    if (!(Ofile = fopen(Header_only ? "yylex.h" : "yylex.c", "w"))) {
      cmd_line_error(0, "can't open output file yylex.[ch]");
    }
  } else {
    Ofile = stdout;
  }

  do_file();
  fclose(Ofile);
  fclose(Ifile);
  exit(0);
}