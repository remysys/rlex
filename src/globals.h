#ifndef _GLOBALS_H
#define _GLOBALS_H

#define MAXINP	2048			        /* maximum rule size		   */

extern int  Verbose;	        	      /* print statistics		   */
extern int  No_lines;         	      /* suppress #line directives	   */
extern int  Public;	          	      /* make static symbols public    */
extern char *Template;         /* state-machine driver template */
extern int  Actual_lineno;    	      /* current input line number	   */
extern int  Lineno;	         	      /* line number of first line of  a multiple-line rule.	   */
extern char Input_buf[MAXINP];		        /* line buffer for input	   */
extern char *Input_file_name;		        /* input file name (for #line   */
extern FILE *Ifile;			                /* input stream.		   */
extern FILE *Ofile;			                /* output stream.		   */
#endif