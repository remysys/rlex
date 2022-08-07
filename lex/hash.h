#ifndef __HASH_H

typedef struct BUCKET {
  struct BUCKET *next;
  struct BUCKET **prev;
} BUCKET;

typedef struct hash_tab_ {
  int size;
  int numsyms;
  unsigned int (*hash) (void *);
  int (*cmp)(void *, void *);
  BUCKET *table[1];
} HASH_TAB;

typedef void (*ptab_t) (void *, ...);

HASH_TAB *maketab(unsigned int maxsym, unsigned int (*hash) (), int (*cmp)());

void *newsym(unsigned int size);
void freesym(void *sym);

void *addsym(HASH_TAB *tabp, void *sym);
void *findsym(HASH_TAB *tabp, void *sym);

void *nextsym(HASH_TAB *tabp, void *i_last);
void delsym(HASH_TAB *tabp, void *sym);
int ptab(HASH_TAB *tabp, ptab_t print, void *param, int sort);

unsigned int hash_add(unsigned char *name);

#endif
