#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <compiler.h>
#include <set.h>
#include <hash.h>
#include "globals.h"
#include "nfa.h"

/* make an nfa from a lex input file using thompson's construction */

typedef enum ERR_NUM 
{
  E_MEM,      /* out of memory                         */
  E_BADEXPR,  /* malformed regular expression          */
  E_PAREN,    /* missing close parenthesis             */
  E_STACK,    /* internal error: discard stack full    */
  E_LENGTH,   /* too many regular expressions          */
  E_BRACKET,  /* missing [ in character class          */
  E_BOL,      /* ^ must be at start of expr or ccl     */
  E_CLOSE,    /* + ? or * must follow expression       */
  E_STRINGS,  /* too many characters in accept actions */
  E_NEWLINE,  /* newline in quoted string              */
  E_BADMAC,   /* missing } in macro expansion          */
  E_NOMAC,    /* macro doesn't exist                   */
  E_MACDEPTH  /* macro expansions nested too deeply    */
} ERR_NUM;

char *Errmsgs[] =  /* indexed by ERR_NUM */
{
  "not enough memory for nfa",
  "malformed regular expression",
  "missing close parenthesis",
  "internal error: discard stack full",
  "too many regular expressions or expression too long",
  "missing [ in character class",
  "^ must be at start of expression or after [",
  "+ ? or * must follow an expression or subexpression",
  "too many characters in accept actions",
  "newline in quoted string, use \\n to get newline into expression",
  "missing } in macro expansion",
  "macro doesn't exist",
  "macro expansions nested too deeply"
};

typedef enum WARN_NUM
{
  W_STARTDASH, /* dash at start of character class */
  W_ENDDASH    /* dash at end of character class   */
} WARN_NUM;

char *Warnmsgs[] = /* indexed by WARN_NUM */
{
  "treating dash in [-...] as a literal dash",
  "treating dash in [...-] as a literal dash"
};

NFA *Nfa_states; 
int Nstates = 0;
int Next_alloc;

// stack
#define SSIZE 32
NFA *Sstack[SSIZE];
NFA **Sp = &Sstack[-1];

#define STACK_OK() (INBOUNDS(Sstack, Sp))
#define STACK_USED() ((int)(Sp - Sstack) + 1)
#define CLEAR_STACK() (Sp = Sstack - 1)
#define PUSH(x) (*++Sp = (x))
#define POP() (*Sp--)

void parse_err(ERR_NUM type);
NFA *rule();
void expr(NFA **startp, NFA **endp);
void cat_expr(NFA **startp, NFA **endp);
void factor (NFA **startp , NFA **endp);
void term(NFA **startp, NFA **endp);
void dodash(SET *set);

NFA *new() 
{
  NFA *p;
  static int first_time = 1;
  if (first_time) {
    if (!(Nfa_states = (NFA *)calloc(NFA_MAX, sizeof(NFA)))) {
      parse_err(E_MEM);
    }
    first_time = 0;
    Sp = &Sstack[-1];
  }
  if (++Nstates >= NFA_MAX) {
    parse_err(E_LENGTH);
  }
  
  p = !STACK_OK() ? &Nfa_states[Next_alloc++] : POP();
  p->edge = EPSILON;
  return p;
}

void discard (NFA *nfa)
{
  --Nstates;
  memset(nfa, 0, sizeof(NFA));
  nfa->edge = EMPTY;
  PUSH(nfa);
  if (!STACK_OK()) {
    parse_err(E_STACK);
  }
}

char *save(char *str) 
{
  static int first_time = 1;
  static int *strings; /* place to save accepting strings */
  static int *savep;   /* current position in strings array*/
  static char size[8]; /* query-mode size */
  
  if (first_time) {
    if (!(savep = strings = (int *)malloc(STR_MAX))) {
      parse_err(E_MEM);
    }
    first_time = 0;
  }

  if (!str) { /* query mode, return number of bytes in use */
    sprintf(size, "%ld", (long)(savep - strings));
    return size;
  }

  if (*str == '|') {
    return (char *) (savep + 1);
  }

  *savep++ = Lineno;

  char *textp;
  for (textp = (char *)savep; *str; *textp++ = *str++) {
    if (textp >= (char *)strings + (STR_MAX - 1)) {
      parse_err(E_STRINGS);
    }
  }

  *textp++ = '\0';
  char *startp = (char *)savep;
  int len = textp - startp;
  savep += (len / sizeof(int)) + (len % sizeof(int) != 0);
  return startp;
}


/* MACRO */

#define MACRO_NAME_MAX 64   // maximun name length
#define MACRO_TEXT_MAX 128  // maximum amount if expansion text

typedef struct MACRO
{
  char name[MACRO_NAME_MAX];
  char text[MACRO_TEXT_MAX];
} MACRO;

HASH_TAB *Macros; // symbol table for macro definitions
/* token definitions */

typedef enum TOKEN
{
  EOS = 1,      // end of string
  ANY,          // .
  AT_BOL,       // ^
  AT_EOL,       // $
  CCL_END,      // ]
  CCL_START,    // [
  CLOSE_CURLY,  // }
  CLOSE_PAREN,  // )
  CLOSURE,      // *
  DASH,         // -
  END_OF_INPUT, // EOF
  L,            // literal character
  OPEN_CURLY,   // {
  OPEN_PAREN,   // (
  OPTIONAL,     // ?
  OR,           // |
  PLUS_CLOSE    // +
} TOKEN;

TOKEN Tokmap[] =
{
/*  ^@  ^A  ^B  ^C  ^D  ^E  ^F  ^G  ^H  ^I  ^J  ^K  ^L  ^M  ^N ^O*/
     L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,

/*  ^P  ^Q  ^R  ^S  ^T  ^U  ^V  ^W  ^X  ^Y  ^Z  ^[  ^\  ^] ^^  ^_*/
     L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,

/*  SPACE  !   "   #    $        %   &    '   (                 )           *         +       ,    -      .    / */
      L,   L,  L,  L,   AT_EOL,  L,  L,   L,  OPEN_PAREN,  CLOSE_PAREN,  CLOSURE, PLUS_CLOSE, L,  DASH,  ANY,  L,


/*  0   1   2   3   4   5   6   7   8   9   :   ;   <   = >      ?     */
    L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  OPTIONAL, 

/*  @   A   B   C   D   E   F   G   H   I   J   K   L   M   N O*/
    L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,

/*  P   Q   R   S   T   U   V   W   X   Y   Z     [    \     ]    ^      _  */
    L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L, CCL_START, L,  CCL_END,  AT_BOL,  L,


/*  `   a   b   c   d   e   f   g   h   i   j   k   l   m n   o*/
    L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,

/*  p   q   r   s   t   u   v   w   x   y   z      {        |      }           ~, DEL*/
    L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L, OPEN_CURLY,  OR,  CLOSE_CURLY,  L,  L  
};


char *(*input_func) (void);   /* input function pointer */
char *Input = "";            /* currretn position in input string */
char *S_input;               /* beginning of input string */
TOKEN Current_tok;           /* current token */
int Lexeme;                  /* value associated with LITERAL */

#define MATCH(t)   (Current_tok == (t))

void errmsg(int type, char *table[], char *msgtype) 
{
  char *p;
  fprintf(stderr, "%s (line %d) %s\n", msgtype, Actual_lineno, table[type]);
  
  for (p = S_input; ++p <= Input; putc('-', stderr))
    ;
  fprintf(stderr, "v\n%s\n", S_input);
  exit(1);
}

void warning(WARN_NUM type) 
{
  errmsg((int)type, Warnmsgs, "WARNING");
}   

void parse_err(ERR_NUM type) 
{
  errmsg((int)type, Errmsgs, "ERROR");
}

void new_macro(char *def) 
{
  /* add a new macro to the table. if two macros have the same name, the
   * second one takes precedence. a definition takes the form:
   * name <whitespace> text [<whitespace>]
   * whitespace at the end of the line is ignored.
   */
  
  char *name;  /* name of macro definition */
  char *text;  /* text of macro definition */
  
  static int first_time = 1;
  
  if (first_time) {
    Macros = maketab(31, hash_add, strcmp);
    first_time = 0;
  }

  for (name = def; *def && !isspace(*def); def++)
    ;
  
  if (*def) {
    *def++ = '\0';
  }

  while(isspace(*def)) { /* skip up to macro body */
    def++;
  }
  
  if (!*def) {
    parse_err(E_BADMAC);
  }

  text = def;


  /* strip trailing white space */
  char *edef = NULL;

  while (*def) {
    if (!isspace(*def)) {
      def++;
    } else {
      for (edef = def; isspace(*def); ++def)
        ;
    }
  }

  if (edef) {
    *edef = '\0';
  }

  /* add the macro to the symbol table */

  MACRO *p = (MACRO *) newsym(sizeof(MACRO));
  
  strncpy(p->name, name, MACRO_NAME_MAX);
  strncpy(p->text, text, MACRO_TEXT_MAX);
  addsym(Macros, p);
}

char *get_macro(char **namep) 
{
  /* return a pointer to the contents of a macro having the indicated
   * name. abort with a message if no macro exists. the macro name includes
   * the brackets. *namep is modified to point past the close brace.
   */

  char *p;
  MACRO *macrop;

  if (!(p = strchr(++(*namep), '}'))) {
    parse_err(E_BADMAC);
  }

  *p = '\0';
  if (!(macrop = (MACRO *) findsym(Macros, *namep))) {
    parse_err(E_NOMAC);
  }

  *p++ = '}';
  
  *namep = p; /* *namep is modified to point past the close brace */
  
  return macrop->text;
}

void print_a_macro(MACRO *macrop) 
{
  printf( "%-16s--[%s]--\n", macrop->name, macrop->text);
}

void printmacs() /* print all the macros to stdout */
{
  if (!Macros) {
    printf("\tthere are no macros\n");
  } else {
    printf("\nMACROS:\n");
    ptab(Macros, (void (*)(void *, ...))print_a_macro, NULL, 1);
  }
}

/*----------------------------------------------------------------
 * lexical analyzer:
 *
 * lexical analysis is trivial because all lexemes are single-character values.
 * the only complications are escape sequences and quoted strings, both
 * of which are handled by advance(), below. this routine advances past the
 * current token, putting the new token into Current_tok and the equivalent
 * lexeme into Lexeme. if the character was escaped, lexeme holds the actual
 * value. for example, if a "\s" is encountered, lexeme will hold a space
 * character.  the MATCH(x) macro returns true if x matches the current token.
 * advance both modifies Current_tok to the current token and returns it.
 *
 * macro expansion is handled by means of a stack (declared at the top
 * of the subroutine). when an expansion request is encountered, the
 * current input buffer is stacked, and input is read from the macro
 * text. this process repeats with nested macros, so SSIZE controls
 * the maximum macro-nesting depth.
 */

 TOKEN advance ()
 {
  static int inquote = 0;     /* processing quoted string */
  int saw_esc;                /* saw a backslash */
  static char *stack[SSIZE];  /* input-source stack */
  static char **sp = NULL;    /* stack pointer */

  if (!sp) {
    sp = &stack[-1];
  } 

  if (Current_tok == EOS) {
    if (inquote) {
      parse_err(E_NEWLINE); 
    }
    do {
    /* sit in this loop until a non-blank line is read into the "Input" array */

      if (!(Input = input_func())) { /* then at end of file  */
        Current_tok = END_OF_INPUT;
        goto exit;
      }

      while (isspace(*Input)) { /* ignore leading white space */
        Input++;
      }
    } while (!*Input);  /* ignore blank lines */

    S_input = Input;    /* remember start of line */
  }

  while (*Input == '\0') {
    if (INBOUNDS(stack, sp)) { /* restore previous input source */
      Input = *sp--;
      continue;
    }

    Current_tok = EOS;  /* no more input sources to restore and at the real end of string*/
    Lexeme = '\0';
    goto exit;
  }

  if (!inquote) {
    while(*Input == '{') {
      *++sp = Input;          /* stack current input string */
      Input = get_macro(sp);  /* use macro body as input string, *sp is modified past the close brace */
      
      if (TOOHIGH(stack, sp)) {
        parse_err(E_MACDEPTH);
      }
    }
  }

  if (*Input == '"') {
    inquote = ~inquote;
    if (!*++Input) {
      Current_tok = EOS;
      Lexeme = '\0';
      goto exit;
    }
  }

  saw_esc = (*Input == '\\');

  if (!inquote) {
    if (isspace(*Input)) {
      Current_tok = EOS;
      Lexeme = '\0';
      goto exit;
    }
    Lexeme = esc(&Input);
  } else {
    if (saw_esc && Input[1] == '"') {
      Input += 2;
      Lexeme = '"';
    } else {
      Lexeme = *Input++;
    }
  }

  Current_tok = (inquote || saw_esc) ? L : Tokmap[Lexeme];

exit:
  return Current_tok;
}

/*--------------------------------------------------------------
 * the parser:
 * a simple recursive descent parser that creates a thompson NFA for
 * a regular expression. the access routine [thompson()] is at the
 * bottom. the NFA is created as a directed graph, with each node
 * containing pointer's to the next node. since the structures are
 * allocated from an array, the machine can also be considered
 * as an array where the state number is the array index.
 */


NFA *machine() 
{
  NFA *start, *p;

  p = start = new();
  p->next = rule();

  while (!MATCH(END_OF_INPUT)) {
    p->next2 = new();
    p = p->next2;
    p->next = rule();
  }

  return start;
}

NFA *rule() 
{
  /*rule --> expr  EOS action
   *      ^expr EOS action
   *      expr$ EOS action
   *
   *action --> <tabs> <string of characters>
   *      epsilon
   *  action  --> white_space string
   *          white_space
   *          epsilon    
   */

  NFA *start = NULL;
  NFA *end = NULL;
  int anchor = NONE;
    
  if (MATCH(AT_BOL)) {
    start = new();
    start->edge = '\n';
    anchor |= START;
    advance();
    expr(&start->next, &end);
  } else {
    expr(&start, &end);
  }

  if (MATCH(AT_EOL)) {
    advance();
    end->next = new();
    end->edge = '\n';
    end = end->next;
    anchor |= END;
  }

  while (isspace(*Input)) {
    Input++;
  }

  end->accept = save(Input);
  end->anchor = anchor;
  advance(); /* skip past EOS */

  return start;
}

void expr (NFA **startp, NFA **endp) 
{
  /* because a recursive descent compiler can't handle left recursion,
   * the productions:
   *
   * expr -> expr OR cat_expr
   *   |  cat_expr
   *
   * must be translated into:
   *
   * expr -> cat_expr expr'
   * expr'-> OR cat_expr expr'
   *   | epsilon
   *
   * which can be implemented with this loop:
   *
   * cat_expr
   * while( match(OR) )
   *  cat_expr
   *  do the OR
   */

  NFA *e2_start;
  NFA *e2_end;
  NFA *p;

  cat_expr(startp, endp);
  while (MATCH(OR)) {
    advance();
    cat_expr(&e2_start, &e2_end);
    p = new();
    p->next = *startp;
    p->next2 = e2_start;
    *startp = p;

    p = new();
    (*endp)->next = p;
    e2_end->next = p;
    *endp = p;
  }
}

int first_in_cat(TOKEN tok) {
  switch (tok) {
    case CLOSE_PAREN:
    case AT_EOL: 
    case OR:
    case EOS: return 0;  
  }

  return 1;
}

void cat_expr(NFA **startp, NFA **endp)
{
  /* the same translations that were needed in the expr rules are needed again
   * here:
   *
   * cat_expr  -> cat_expr factor
   *       | factor
   *
   * is translated to:
   *
   * cat_expr  -> factor cat_expr'
   * cat_expr' -> factor cat_expr'
   *       | epsilon
   */

  NFA *e2_start;
  NFA *e2_end;

  if (first_in_cat(Current_tok)) {
    factor(startp, endp);
  }

  while (first_in_cat(Current_tok)) {
    factor(&e2_start, &e2_end);
    memcpy(*endp, e2_start, sizeof(NFA));
    discard(e2_start);
    *endp = e2_end;
  }
}


void factor (NFA **startp , NFA **endp) {

  /*  factor --> term*  | term+  | term? | term */
  NFA *start;
  NFA *end;
  term(startp, endp);
  if (MATCH(CLOSURE) || MATCH(PLUS_CLOSE) || MATCH(OPTIONAL)) {
    start = new();
    end = new();
    start->next = *startp;
    (*endp)->next = end;
      
    if (MATCH(CLOSURE) || MATCH(OPTIONAL)) {
      start->next2 = end;
    }

    if (MATCH(CLOSURE) || MATCH(PLUS_CLOSE)) {
      (*endp)->next2 = *startp;
    }

    *startp = start;
    *endp = end;
    advance();
  }
}

void term(NFA **startp, NFA **endp) {
  /* process the term productions:
   *
   * term  --> [string]  |  [^string]  |  []  |  [^] |  .  | (expr) | <character>
   *
   * the [] is nonstandard. It matches a space, tab, formfeed, or newline,
   * but not a carriage return (\r). All of these are single nodes in the
   * NFA.
   */ 
  NFA *start;
  int c;

  if (MATCH(OPEN_PAREN)) {
    advance();
    expr(startp, endp);
    if (MATCH(CLOSE_PAREN)) {
      advance();
    } else {
      parse_err(E_PAREN);
    }
  } else {
    *startp = start = new();
    *endp = start->next = new();

    if (!(MATCH(ANY) || MATCH(CCL_START))) {
      start->edge = Lexeme;
      advance();
    } else {
      start->edge = CCL;
      if (!(start->bitset = newset())) {
        parse_err(E_MEM);
      }

      if (MATCH(ANY)) {
        ADD(start->bitset, '\n');
        COMPLEMENT(start->bitset);
      } else {
        advance();
        if (MATCH(AT_BOL)) {
          advance();
          ADD(start->bitset, '\n');
          COMPLEMENT(start->bitset);
        }

        if (!MATCH(CCL_END)) {
          dodash(start->bitset);
        } else { /* [] [^] */
          for (c = 0; c <= ' '; c++) {
            ADD(start->bitset, c);
          }
        }
      }
      advance();
    }
  }
}

void dodash(SET *set) 
{
  int first;

  if (MATCH(DASH)) { /* treat [-...] as a literal dash */
    warning(W_STARTDASH);
    ADD(set, Lexeme);
    advance();
  }

  for (; !MATCH(EOS) && !MATCH(CCL_END); advance()) {
    if (!MATCH(DASH)) {
      first = Lexeme;
      ADD(set, Lexeme);
    } else {
      advance();
      if (MATCH(CCL_END)) { /* treat [...-] as literal */
        warning(W_ENDDASH);
        ADD(set, Lexeme);
      } else {
        for (; first <= Lexeme; first++) {
          ADD(set, first);
        }
      }
    }
  }
}
 

NFA *thompson(char *(*input_function)(void), int *max_state, NFA **start_state)
{
  /* access routine to this module. return a pointer to a NFA transition
   * table that represents the regular expression pointed to by expr or
   * NULL if there's not enough memory. modify *max_state to reflect the
   * largest state number used. this number will probably be a larger
   * number than the total number of states. modify *start_state to point
   * to the start state. this pointer is garbage if thompson() returned 0.
   * the memory for the table is fetched from malloc(); use free() to
   * discard it.
   */

  CLEAR_STACK();
  input_func = input_function;

  Current_tok = EOS;
  advance();

  Nfa_states = 0;
  Next_alloc = 0;
    
  *start_state = machine();
  *max_state = Next_alloc;

  if (Verbose > 1) {
    print_nfa(Nfa_states, *max_state, *start_state);
  }

  if (Verbose) {
    printf("%d/%d NFA states used.\n", *max_state, NFA_MAX );
    printf("%s/%d bytes used for accept strings.\n\n", save(NULL), STR_MAX);
  }

  return Nfa_states;
}

#ifdef MAIN

char *getstr()
{
  static char bufs[80];
  printf("%d: ", Lineno++);
  if (fgets(bufs, NUMELE(bufs), stdin)) {
    return bufs;
  } else {
    return NULL;
  }
}

int main(int argc, char *argv[])
{
  NFA *nfa, *start_state;
  int max_state;
  Verbose = 2;

  nfa = thompson(getstr, &max_state, &start_state);
  return 0;
}
#endif