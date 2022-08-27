#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <compiler.h>

void *newsym(unsigned int size) 
{
  BUCKET *sym = (BUCKET *)calloc(1, size + sizeof(BUCKET));
  
  if (!sym) {
    fprintf(stderr, "can't get memory\n");
    exit(1);
    return NULL;
  }

  return (void *)(sym + 1);
} 

void freesym(void *sym) 
{
  free((BUCKET *)sym -1);
}


HASH_TAB *maketab(unsigned int maxsym, unsigned int (*hash)(), int (*cmp)()) 
{
  if (!maxsym) {
    maxsym = 127;
  }

  HASH_TAB *p = (HASH_TAB *) calloc (1, maxsym * sizeof(BUCKET *) + sizeof(HASH_TAB));
  if (!p) {
    fprintf(stderr, "can't get memory\n");
    exit(1);
  }
  p->size = maxsym;
  p->numsyms = 0;
  p->hash = (unsigned int (*)(void *))hash;
  p->cmp = (int (*)(void*, void*))cmp;

  return p;
}


void *addsym(HASH_TAB *tabp, void *isym)
{
  BUCKET *sym = (BUCKET *)isym;
  BUCKET **p = &(tabp->table)[tabp->hash(sym--) % tabp->size];
  
  BUCKET *tmp = *p;
  *p = sym;
  sym->prev = p;
  sym->next = tmp;

  if (tmp) {
    tmp->prev = &sym->next;
  }

  tabp->numsyms++;
  return (void *)(sym + 1);
}

void delsym (HASH_TAB *tabp, void *isym)
{
  if (tabp && isym) {
    BUCKET *sym = (BUCKET *)isym - 1;
    --tabp->numsyms;

    if (*(sym->prev) = sym->next) {
      sym->next->prev = sym->prev;
    }
  }
}

void *findsym(HASH_TAB *tabp, void *sym) {
  if (!tabp) {
    return NULL;
  }
  
  BUCKET *p = (tabp->table)[ tabp->hash(sym) % tabp->size];

  while (p && (tabp->cmp)(sym, p + 1)) {
    p = p->next;
  }

  return (void *) p ? p + 1 : NULL; 
}

void *nextsym(HASH_TAB *tabp, void *i_last)
{
  BUCKET *last = (BUCKET *)i_last;
  for (--last; last->next; last = last->next) {
    if (tabp->cmp(i_last, last->next + 1) == 0) {
      return (void *) (last->next + 1);
    }
  }
  return NULL;
}

int (*user_cmp)(void *, void *);

int internal_cmp(const void *p1, const void *p2)
{
  return (user_cmp)((void*)(*(BUCKET **)p1 + 1), (void*)(*(BUCKET **)p2 + 1));
}

int ptab(HASH_TAB *tabp, ptab_t print, void *param, int sort) 
{
  BUCKET **symtab;
  BUCKET **outtab;
  BUCKET *sym;
  int i;

  if (!tabp || tabp->size == 0) {
    return 1;
  }

  if (!sort) {  
    for (symtab = tabp->table, i = tabp->size; --i >= 0; symtab++) {
      for (sym = *symtab; sym; sym = sym->next) {
        print(sym + 1, param);
      }
    }
  } else {
    outtab = (BUCKET **) malloc (tabp->numsyms * sizeof (BUCKET *));
    if (!outtab) {
      fprintf(stderr, "can't get memory\n");
      return 0;
    }

    BUCKET **outp = outtab;
    for (symtab = tabp->table, i = tabp->size; --i >= 0; symtab++) {
      for (sym = *symtab; sym; sym = sym->next) {
        if (outp >= outtab + tabp->numsyms) {
          fprintf(stderr, "internal error\n");
          exit(1);
        }
        *outp++ = sym;
      }
    }

    user_cmp = tabp->cmp;
    qsort(outtab, tabp->numsyms, sizeof(BUCKET *), internal_cmp);

    for (outp = outtab, i = tabp->numsyms; --i >= 0; outp++) {
      print(*(outp) + 1, param);
    }

    free(outtab);
  }

  return 1;
}

unsigned int hash_add(unsigned char *name)
{
  unsigned int h ;
  for(h = 0; *name ; h += *name++)
    ;
  return h;
}

#ifdef  MAIN
/*----------------------------------------------------------------------
 * The following test routines exercise the hash functions by building a table
 * consisting of either 500 words comprised of random letters, result are 
 * then printed.
 */

#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>

typedef struct
{
  char          name[32];	/* hash key */
  char          str[16];  /* used for error checking */
  unsigned int  hval;	    /* hash value of name, also "	*/
  int	          count;	  /* # of times word was encountered */
} STAB;

void printword(void *sp, ...) {
  printf("name: %s\tstr: %s\thval: %u\n", ((STAB *)sp)->name, ((STAB *)sp)->str, ((STAB *)sp)->hval);
}

int getword(char *buf)
{
 /* generate 10 random words */

	static int	wordnum = 10;
	int	num_letters, let;

	if(--wordnum < 0)
	    return 0;

	while((num_letters = rand() % 16)  < 3)
	    ;

	while(--num_letters >= 0) {
    let = (rand() % 26) + 'a' ;	/* 26 letters in english */
    *buf++ = (rand() % 10) ? let : toupper(let) ;
	}

	*buf = '\0';
	return 1;
}

int main(int argc, char *argv[])
{
    char word[80];
    STAB	*sp;
    HASH_TAB	*tabp;
    int		 c;

    tabp = maketab(127, hash_add, strcmp );

    while (getword(word)) {
	    if (sp = (STAB *) findsym(tabp, word)) {
	      if (strcmp(sp->str,"123456789abcdef") || (sp->hval != hash_add(word))) {
          printf("NODE HAS BEEN ADULTERATED\n");
          exit( 1 );
	      }
	      sp->count++;
	    } else {
          sp = newsym(sizeof(STAB));
          strncpy(sp->name, word, 32);
          strcpy (sp->str, "123456789abcdef");
          sp->hval = hash_add(word);
          addsym(tabp, sp);
          sp->count = 1;
      }
    }
    ptab(tabp, printword, NULL, 1);
}
#endif



