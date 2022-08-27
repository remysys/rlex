#include <stdio.h>
#include <ctype.h>
#include <compiler.h>

/* map escape sequences to single characters */

#define ISHEXDIGIT(x) (isdigit(x) || ('a' <= (x) && (x) <= 'f') || ('A' <= (x) && (x) <= 'F'))
#define ISOCTDIGIT(x) ('0' <= (x) && (x) <= '7')

int hex2bin(int c) 
{
  /* convert the hex digit represented by 'c' to an int. 
   * 'c' must be one of the following characters: 0123456789abcdefABCDEF
   */

  return (isdigit(c) ? (c) - '0' : (toupper(c) - 'A') + 10) & 0xf;
}

int oct2bin(int c) 
{
  /* convert the hex digit represented by 'c' to an int. 
   *'c' must be a digit in the range '0'-'7'.
   */
  return ((c) - '0') & 0x7;
}

int esc(char **s) 
{
  /* map escape sequences into their equivalent symbols. Return the equivalent
   * ASCII character. *s is advanced past the escape sequence. If no escape
   * sequence is present, the current character is returned and the string
   * is advanced by one. The following are recognized:
   *
   *	\b	backspace
   *	\f	formfeed
   *	\n	newline
   *	\r	carriage return
   *	\s	space
   *	\t	tab
   *	\e	ASCII ESC character ('\033')
   *	\DDD	number formed of 1-3 octal digits
   *	\xDDD	number formed of 1-3 hex digits
   *	\^C	C = any letter. Control code
   */

  int rval;
  if (**s != '\\') {
    rval = *((*s)++);
  } else {
    ++(*s); /* skip the \ */
    switch (toupper(**s)) {
      case '\0':  rval = '\\';    break;
      case 'B':   rval = '\b';    break;
      case 'F':   rval = '\f';    break;
      case 'N':   rval = '\n';    break;
      case 'R':   rval = '\r';    break;
      case 'S':   rval = ' ';     break;
      case 'T':   rval = '\t';    break;
      case 'E':   rval = '\033';  break;

      case '^':   rval = *++(*s);
                  rval = toupper(rval) - '@';
                  break;
      case 'X':   rval = 0;
                  ++(*s);
                  if (ISHEXDIGIT(**s)) {
                    rval = hex2bin(*(*s)++);
                  }

                  if (ISHEXDIGIT(**s)) {
                    rval <<= 4;
                    rval |= hex2bin(*(*s)++);
                  }

                  if (ISHEXDIGIT(**s)) {
                    rval <<= 4;
                    rval |= hex2bin(*(*s)++);
                  }

                  --(*s);
                  break;
      default:    if (!ISOCTDIGIT(**s)) {
                    rval = **s;
                  } else {
                    rval = oct2bin(*(*s)++);
                    if (ISHEXDIGIT(**s)) {
                      rval <<= 3;
                      rval |= oct2bin(*(*s)++);
                    }
                    if (ISOCTDIGIT(**s)) {
                      rval <<= 3;
                      rval |= oct2bin(*(*s)++);
                    }
                    --(*s);
                  }
                  break;

    }
    ++(*s);
  }

  return rval;
}

char *bin_to_ascii(int c, int use_hex)
{
  /* return a pointer to a string that represents c. this will be the
   * character itself for normal characters and an escape sequence (\n, \t,
   * \x00, etc., for most others). A ' is represented as \'. the string will
   * be destroyed the next time bin_to_ascii() is called. ff "use_hex" is true
   * then \xDD escape sequences are used. otherwise, octal sequences (\DDD)
   * are used
   */

  static char buf[8];
  c  &= 0xff;
  if (' ' <= c && c < 0x7f && c != '\'' && c != '\\') {
    buf[0] = c;
    buf[1] = '\0';
  } else {
    buf[0] = '\\';
    buf[2] = '\0';
    
    switch (c) {
      case '\\': buf[1] = '\\'; break;
      case '\'': buf[1] = '\''; break;
      case '\b': buf[1] = 'b' ; break;
      case '\f': buf[1] = 'f' ; break;
      case '\t': buf[1] = 't' ; break;
      case '\r': buf[1] = 'r' ; break;
      case '\n': buf[1] = 'n' ; break; 
      default: sprintf(&buf[1], use_hex? "x%03x" : "%03o", c); break;
    }
  }

  return buf;
}
