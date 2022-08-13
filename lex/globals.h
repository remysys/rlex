#ifndef _GLOBALS_H

#define MAXINP	2048			        /* maximum rule size		   */

int  Verbose	        = 0 ;	      /* print statistics		   */
int  No_lines         = 0 ;	      /* suppress #line directives	   */
int  Unix		          = 0 ;	      /* use UNIX-style newlines       */
int  Public	          = 0 ;	      /* make static symbols public    */
char *Template        ="lex.par"; /* state-machine driver template */
int  Actual_lineno    = 1 ;	      /* current input line number	   */
int  Lineno	          = 1 ;	      /* line number of first line of  a multiple-line rule.	   */
char Input_buf[MAXINP];		        /* line buffer for input	   */
char *Input_file_name;		        /* input file name (for #line   */
FILE *Ifile;			                /* input stream.		   */
FILE *Ofile;			                /* output stream.		   */
#endif