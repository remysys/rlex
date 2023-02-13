#ifndef _SET_H
#define _SET_H

#define _BITS_IN_INT 32         /* bits in one cell */
#define _DEFSIZE 4              /* cells in default set */
#define _DEFBITS  (_DEFSIZE * _BITS_IN_INT)  /* bits in default set */

typedef struct _set_
{
  unsigned int nsize;            /* # size of map */
  unsigned char compl;           /* is a negative true set if true */
  unsigned int nbits;            /* number of bits in map */
  unsigned int *map;             /* pointer to the map */
  unsigned int defmap[_DEFSIZE]; /* the map itself */
} SET;

typedef int (*pset_t) (void* param, char *fmt, ...);
int _addset(SET* , int );
void delset(SET*);
SET *dupset(SET*);
void invert(SET*);
SET *newset();
int next_member(SET*);
int num_ele(SET*);
void pset(SET*, pset_t, void*); /* pset(set, fprintf, stdout ); */
void _set_op(int, SET*, SET*);
int _set_test(SET*, SET*);
int setcmp(SET*, SET*);
unsigned int sethash(SET*);
int subset(SET*, SET*);
void truncate(SET*);

/* op argument passed to _set_op */
#define _UNION      0  
#define _INTERSECT  1
#define _DIFFERENCE 2
#define _ASSIGN     3 

#define UNION(d, s)       _set_op(_UNION, d, s)         //d or s
#define INTERSECT(d, s)   _set_op(_INTERSECT, d, s)     // d and s
#define DIFFERENCE(d, s)  _set_op(_DIFFERENCE, d, s) 
#define ASSIGN(d, s)      _set_op(_ASSIGN, d, s)


#define CLEAR(s)        memset((s)->map, 0, (s)->nsize * sizeof(unsigned int))
#define FILL(s)         memset((s)->map, ~0, (s)->nsize * sizeof(unsigned int))
#define COMPLEMENT(s)   ((s)->compl = ~((s)->compl))
#define INVERT(s)       invert(s)

#define _SET_EQUIV  0    /* value returned from _set_test, equivalent */
#define _SET_DISJ   1   
#define _SET_INTER  2

#define IS_DISJOINT(s1, s2)     (_set_test(s1, s2) == _SET_DISJ)
#define IS_INTERSECTING(s1, s2) (_set_test(s1, s2) == _SET_INTER)
#define IS_EQUIVALENT(a, b)     (setcmp(a, b) == 0)
#define IS_EMPTY(s)             (num_ele(s) == 0)

#define _DIV_INDEX(x) ((unsigned int)(x) >> 5)
#define _MOD_VAL(x) ((x) & 0x1f)
#define _ROUND(bit) (((_DIV_INDEX(bit) + 4) >> 2) << 2) 

#define _GBIT(s, x, op) (((s)->map)[_DIV_INDEX(x)] op (1 << _MOD_VAL(x)))

#define REMOVE(s, x)    (((x) >= (s)->nbits) ? 0 : _GBIT(s, x, &= ~))
#define ADD(s, x)       (((x) >= (s)->nbits) ? _addset(s, x) : _GBIT(s, x, |=))
#define MEMBER(s, x)    (((x) >= (s)->nbits) ? 0 : _GBIT(s, x, &))
#define TEST(s, x)      (MEMBER(s, x)        ? !(s)->compl : (s)->compl)

#endif