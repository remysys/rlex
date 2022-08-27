#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "l.h"

/* input.c: the input system used by lex-generated lexical analyzers */

#define STDIN 0

#define MAXLOOK 16    /* maximum amount of lookahead */
#define MAXLEX  1024  /* maximum lexme sizes */

#define BUFSIZE ((MAXLEX * 3) + (2 * MAXLOOK))
#define DANGER  (End_buf - MAXLOOK)     /* flush bufffer when next passes this address */

#define END (&Start_buf[BUFSIZE])

#define NO_MORE_CHARS() (Eof_read && Next >= End_buf)


char Start_buf[BUFSIZE]; /* input buffer */
char *End_buf  = END;    
char *Next     = END;
char *sMark    = END;    /* start of current lexeme */
char *eMark    = END;    /* end of current lexeme */

char *pMark    = NULL;   /* start of previous lexeme */
int pLineno     = 0;      /* line # of previous lexeme */
int pLength     = 0;      /* length of previous lexeme */

int Inp_file    = STDIN;
int Lineno      = 1;      /* current line number */
int Mline       = 1;      /* line # when mark_end() called */
int Termchar    = 0;     
int Eof_read    = 0;      /* end of file has been read. it's possible for this to be true and
                           * for characters to still be in the input buffer */
                    
/* pointers to open, close, and read functions */

int (*Openp) (char *, int ) = (int (*)(char *, int)) open;
int (*Closep) (int) = close;
int (*Readp)(int, void *, unsigned int) = (int (*)(int, void *, unsigned int))read;

void ii_io(int (*open_funct)(char *, int), 
  int (*close_funct)(int), 
  int (*read_funct)(int, void *, unsigned int))
{
  Openp = open_funct;
  Closep = close_funct;
  Readp = read_funct;
}

int ii_newfile(char *name)
{
  int fd;

  if (!name) {
    fd = STDIN;
  } else {
    fd = Openp(name, O_RDONLY);
  }

  if (fd != -1) {
    if (Inp_file != STDIN) {
      Closep(Inp_file);
    }

    Inp_file = fd;
    Eof_read = 0;
    Next  = END;
    sMark = END;
    pMark = END;
    eMark = END;
    End_buf = END;
    Lineno = 1;
    Mline = 1;
  }

  return fd;
}

char *ii_text() 
{
  return sMark; 
}

int ii_length()
{
  return eMark - sMark;
}

int ii_lineno()
{
  return Lineno;
}

char *ii_ptext()
{
  return pMark;
}

int ii_plength()
{
  return pLength;
}

int ii_plineno()
{
  return pLineno;
}

char *ii_mark_start()
{
  Mline = Lineno;
  eMark = sMark = Next;
  return sMark;
}

char *ii_mark_end()
{
  Mline = Lineno;
  return (eMark = Next);
}

char *ii_move_start()
{
  if (sMark >= eMark) {
    return NULL;
  } else {
    return ++sMark;
  }
}

char *ii_to_mark()
{
  Lineno = Mline;
  return (Next = eMark);
}

char *ii_mark_prev()
{
  pLineno = Lineno;
  pLength = eMark - sMark;
  return (pMark = sMark);
}

int ii_advance()
{
  /* ii_advance() is the real input function. it returns the next character
   * from input and advances past it. the buffer is flushed if the current
   * character is within MAXLOOK characters of the end of the buffer. 0 is
   * returned at end of file. -1 is returned if the buffer can't be flushed
   * because it's too full. in this case you can call ii_flush(1) to do a
   * buffer flush but you'll loose the current lexeme as a consequence.
   */

  static int been_called = 0;
  if (!been_called) {
    /* push a newline into the empty buffer so that the lex start-of-line
	   * anchor will work on the first input line.
	   */
    Next = sMark = eMark = END - 1;
    pMark = NULL;
    pLength = 0;
    *Next = '\n';
    --Lineno; 
    --Mline;
    been_called = 1;
  }

  if (NO_MORE_CHARS()) {
    return 0;
  }

  if (!Eof_read && ii_flush(0) < 0) {
    return -1;
  }

  if (*Next == '\n') {
    Lineno++;
  }

  return *Next++;
}

int ii_flush(int force)
{
  /* either the pMark or sMark (whichever is smaller) is used as the leftmost
   * edge of the buffer. none of the text to the right of the mark will be
   * lost. return 1 if everything's ok, -1 if the buffer is so full that it
   * can't be flushed. 0 if we're at end of file. if "force" is true, a buffer
   * flush is forced and the characters already in it are discarded. don't
   * call this function on a buffer that's been terminated by ii_term().
   */
  
  int copy_amt, shift_amt;
  char *left_edge;

  if (NO_MORE_CHARS()) {
    return 0;
  }

  if (Eof_read) {
    return 1;
  }

  if (Next >= DANGER || force) {
    left_edge = pMark ? pMark : sMark;
    shift_amt = left_edge - Start_buf;

    if (shift_amt < MAXLEX) { /* if (not enough room) */
      if (!force) {
        return -1;
      }
      left_edge = ii_mark_start(); /* discard current lexeme and previous lexeme */
      ii_mark_prev();
      shift_amt = left_edge - Start_buf;
    }

    copy_amt = End_buf - left_edge;
    memmove(Start_buf, left_edge, copy_amt); /* memmove: the destination can overlap the source */

    if (!ii_fillbuf(Start_buf + copy_amt)) {
      ferr("internal error, ii_flush: buffer full, can't read\n");
    }

    if (pMark) {
      pMark = pMark - shift_amt;
    }

    sMark -= shift_amt;
    eMark -= shift_amt;
    Next  -= shift_amt;
  }

  return 1;
}

int ii_fillbuf(char *starting_at)
{
  /* fill the input buffer from starting_at to the end of the buffer.
   * The input file is not closed when EOF is reached. fuffers are read
   * in units of MAXLEX characters; it's an error if that many characters
   * cannot be read (0 is returned in this case). for example, if MAXLEX
   * is 1024, then 1024 characters will be read at a time. The number of
   * characters read is returned. Eof_read is true as soon as the last
   * buffer is read.
   */

  int need;
  int got;
   
  need = ((END - starting_at) / MAXLEX) * MAXLEX;
   
  if (need < 0) {
    ferr("internal error (ii_fillbuf): bad read-request starting addr\n");
  }

  if (need == 0) {
    return 0;
  }

  if ((got = Readp(Inp_file, starting_at, need)) < 0) {
    ferr("can't read input file\n");
  }

  End_buf = starting_at + got;

  if (got < need) {
    Eof_read = 1;
  }

  return got;
}

int ii_look(int n) 
{
  /* return the nth character of lookahead, EOF if you try to look past
   * end of file, or 0 if you try to look past either end of the buffer.
   */
  
  if (n > (End_buf - Next)) { /* (End_buf - Next) is the # of unread */
    return Eof_read ? EOF : 0; 
  }

  /* the current lookahead character is at Next[0]. the last character */
  /* read is at Next[-1]. The --n in the following if statement adjusts */
  /* n so that Next[n] will reference the correct character*/

  if (--n < - (Next - Start_buf)) { /* (Next - Start_buf) is the # of buffered */ 
    return 0;  /* look past, characters that have been read */
  }

  return Next[n];
}

int ii_pushback(int n)
{
  /* push n characters back into the input. you can't push past the current
   * sMark. you can, however, push back characters after end of file has
   * been encountered. if you try to push past the sMark, only the characters 
   * as far as the sMark are pushed and 0 is returned (1 is returned on a successful push).
   */
  
  while (--n >= 0 && Next > sMark) {
    if (*--Next == '\n') {
      --Lineno;
    }
  }
  
  if (Next < eMark) {
    eMark = Next;
    Mline = Lineno;
  }

  return (Next > sMark);
}

void ii_term()
{
  Termchar = *Next;
  *Next = '\0';
}

void ii_unterm()
{
  if (Termchar) {
    *Next = Termchar;
    Termchar = 0;
  }
}

int ii_input()
{
  int rval;

  if (Termchar) {
    ii_unterm();
    rval = ii_advance();
    ii_mark_end();
    ii_term();
  } else {
    rval = ii_advance();
    ii_mark_end();
  }

  return rval;
}

void ii_unput(int c)
{
  if (Termchar) {
    ii_unterm();
    if (ii_pushback(1)) {
      *Next = c;
    }
    ii_term();
  } else {
    if (ii_pushback(1)) {
      *Next = c;
    }
  }
}

int ii_lookahead(int n) {
  return (n == 1 && Termchar) ? Termchar : ii_look(n);
}

int ii_flushbuf()
{
  if (Termchar) {
    ii_unterm();
  }
  return ii_flush(1);
}
