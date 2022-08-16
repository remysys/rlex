#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "comm.h"
#include "dfa.h"
#include "globals.h"
#include "set.h"

/* squash.c -- this module contains the routines to compress a table
 * horizontally and vertically by removing redundant columns and rows, and then
 * print the compressed table.
 */


#define NCOLS	16
#define TYPE	 "YY_TTYPE"  /* Declared type of output tables.		*/
#define SCLASS   "YYPRIVATE" /* Storage class of all the tables		*/

int Col_map[MAX_CHARS];
int Row_map[DFA_MAX];

#define ROW_EQUIV(r1, r2, ncols) (memcmp(r1, r2, ncols * sizeof(int)) == 0)
#define ROW_CPY(r1, r2, ncols) (memcpy(r1, r2, ncols * sizeof(int)))


int col_equiv(int *col1, int *col2, int nrows) 
{
  /* return 1 if the two columns are equivalent, else return 0 */
  while (--nrows >= 0 && *col1 == *col2) {
    col1 += MAX_CHARS;
    col2 += MAX_CHARS;
  }

  return (!(nrows >= 0));
}

void col_cpy(int *dest, int *src, int nrows, int n_src_cols, int n_dest_cols)
{
  while (--nrows >= 0) {
    *dest = *src;
    dest += n_dest_cols;
    src += n_src_cols;
  }
}

void reduce(ROW *dtran, int *p_nrows, int *p_ncols)
{
  int ncols = *p_ncols;
  int nrows = *p_nrows;

  int r_ncols; /* number of columns in reduced machine */
  int r_nrows; /* number of rows in reduced machine */
  int i;
  int j; 

  SET *save;

  int *current;
  int *compressed;
  int *p;

  memset(Col_map, -1, sizeof(Col_map));
  save = newset();

  for (r_ncols = 0; ;r_ncols++) {
    /* skip past any states in the Col_map that have already been
	    * processed. If the entire Col_map has been processed, break
	    */
    for (i = r_ncols; Col_map[i] == -1 && i < ncols; i++) {
      ;
    }

    if (i >= ncols) {
      break;
    }

    ADD(save, i);
    Col_map[i] = r_ncols;

    current = &dtran[0][i];
    p = current + 1;
    for (j = i; ++j < ncols; p++) {
      if (Col_map[j] != -1 && col_equiv(current, p, nrows)) { /* MAX_CHARS == ncols */
        Col_map[j] = r_ncols;
      }
    }
  }

  compressed = (int *) malloc (nrows * r_ncols * sizeof(int));
  if (!compressed) {
    ferr("can't get memory\n");
  }

  p = compressed;
  for (next_member(NULL); (i = next_member(save)) != -1; ) {
    col_cpy(p++, &dtran[0][i], nrows, ncols, r_ncols);
  }
  
  /* ............................................................
    * eliminate equivalent rows, working on the reduced array
    * created in the previous step. The algorithm used is the same
    */
  
  memset(Row_map, -1, sizeof(Row_map));
  CLEAR(save);
  
  for (r_nrows = 0; ; r_nrows++) {
    for (i = r_nrows; Row_map[i] != -1 && i < nrows; i++) {
      ;
    }

    if (i >= nrows) {
      break;
    }

    ADD(save, i);
    Row_map[i] = r_nrows;

    current = compressed + i * r_ncols;
    p       = compressed + (i + 1) * r_ncols;
    for (j = i; ++j < nrows; p += r_ncols) {
      if (Row_map[j] == -1 && ROW_EQUIV(current, p, r_ncols)) {
        Row_map[j] = r_nrows;
      }
    }
  }

  p = (int*) dtran;
  
  for (next_member(NULL); (i = next_member(save)) != -1; ) {
    ROW_CPY(p, compressed + i * r_ncols, r_ncols);
    p += r_ncols;
  }

  delset(save);
  free(compressed);
  *p_ncols = r_ncols;
  *p_nrows = r_nrows;
}

void pmap(FILE *fp, int *p,  int n )
{
    /*print a one-dimensional array */

  int i;
  for (i = 0; i < (n - 1); i++) {
	  fprintf(fp, "%3d," , *p++);
	  if ((i % NCOLS) == NCOLS - 1) {
     	fprintf(fp, "\n     "); 
    }
  }

  fprintf(fp, "%3d\n}; \n\n", *p);
}

void print_col_map(FILE *fp)
{
  static char	*text[] = {
      "The Yy_cmap[] and Yy_rmap arrays are used as follows:",
      "",
      " next_state= Yydtran[ Yy_rmap[current_state] ][ Yy_cmap[input_char] ];",
      "",
      "Character positions in the Yy_cmap array are:",
      "",
      "   ^@  ^A  ^B  ^C  ^D  ^E  ^F  ^G  ^H  ^I  ^J  ^K  ^L  ^M  ^N  ^O",
      "   ^P  ^Q  ^R  ^S  ^T  ^U  ^V  ^W  ^X  ^Y  ^Z  ^[  ^\\  ^]  ^^  ^_",
      "        !   \"   #   $   %   &   '   (   )   *   +   ,   -   .   /",
      "    0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?",
      "    @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O",
      "    P   Q   R   S   T   U   V   W   X   Y   Z   [   \\   ]   ^   _",
      "    `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o",
      "    p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~   DEL",
      NULL
  };

  comment(fp, text);
  fprintf(fp, "%s %s  Yy_cmap[%d] =\n{\n     ", SCLASS, TYPE, MAX_CHARS);
  pmap(fp, Col_map, MAX_CHARS );
}

void print_row_map(FILE *fp, int nrows )
{
  fprintf(fp, "%s %s  Yy_rmap[%d] =\n{\n     ", SCLASS, TYPE, nrows);
  pmap(fp, Row_map, nrows);
}

int squash(FILE *fp, ROW *dtran, int nrows, int ncols, char *name)
{
  /* compress (and output) dtran using equivalent-column elimination.
   * return the number of bytes required for the compressed tables
   * (including the map but not the accepting array).
   */

  int oncols = ncols;  /* original column count */
  int onrows = nrows;  /* original column count */

  reduce(dtran, &nrows, &ncols);
  print_col_map(fp);
  print_row_map(fp, onrows);

  fprintf(fp, "%s %s %s[ %d ][ %d ]=\n", SCLASS, TYPE, name, nrows, ncols);
  print_array(fp, (int *)dtran, nrows, ncols);

  return (nrows * ncols * sizeof(TTYPE)) /* dtran */ 
        + (onrows * sizeof(TTYPE))      /* row map */
        + (oncols * sizeof(TTYPE));      /* col map */
}

    
  
void	cnext(FILE *fp, char *name)
{
  /* print out a yy_next(state,c) subroutine for the compressed table */

  static char	*text[] = {
    "yy_next(state,c) is given the current state number and input",
    "character and evaluates to the next state.",
    NULL
  };

  comment(fp, text);
  fprintf(fp, "#define yy_next(state,c) (%s[ Yy_rmap[state] ][ Yy_cmap[c] ])\n", name);
}
