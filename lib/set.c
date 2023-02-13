#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <set.h>
#include <compiler.h>

SET *newset() 
{
  SET *p = (SET *) malloc (sizeof(SET));
  if (!p) {
    fprintf(stderr, "can't get memory\n");
    exit(1);
    return NULL;
  }

  memset(p, 0, sizeof(SET));
  p->map = p->defmap;
  p->nsize = _DEFSIZE;
  p->nbits = _DEFBITS; 
  return p;
}

void delset(SET *set) 
{
  if (set->map != set->defmap) {
    free(set->map);
  }
  free(set);
}

SET *dupset(SET *set) 
{
  if (!set) {
    return NULL;
  }

  SET *new_set = (SET *) malloc (sizeof(SET));
  if (!new_set) {
    fprintf(stderr, "can't get memory\n");
    exit(1);
    return NULL;
  }

  memset(new_set, 0, sizeof(SET));
  new_set->compl = set->compl;
  new_set->nsize = set->nsize;
  new_set->nbits = set->nbits;

  if (set->map == set->defmap) {
    new_set->map = new_set->defmap;
    memcpy(new_set->defmap, set->defmap, _DEFSIZE * sizeof(unsigned int));
  } else {
    new_set->map = (unsigned int *) malloc (new_set->nsize * sizeof(unsigned int));
    if (!new_set->map) {
      fprintf(stderr, "can't get memory\n");
      exit(1);
      return NULL;
    }

    memcpy(new_set->map, set->map, new_set->nsize * sizeof(unsigned int));
  }

  return new_set;
}

void enlarge(int need, SET *set) {
  if (!set || need <= set->nsize) {
    return;
  }

  unsigned int *new_map = (unsigned int *) malloc (need * sizeof (unsigned int));
  if (!new_map) {
    fprintf(stderr, "can't get memory\n");
    exit(1);
  }

  memcpy(new_map, set->map, set->nsize * sizeof(unsigned int));
  memset(new_map + set->nsize, 0, (need - set->nsize) * sizeof (unsigned int));

  if (set->map != set->defmap) {
    free(set->map);
  }
  set->map = new_map;
  set->nsize = (unsigned int) need;
  set->nbits = set->nsize * _BITS_IN_INT;
}

int	_addset(SET *set, int bit)
{
  enlarge(_ROUND(bit), set);
  return _GBIT(set, bit, |=);
}

int num_ele(SET *set) 
{
  if (!set) {
    return 0;
  }
  unsigned int count = 0;
  int i;
  for (i = 0; i < set->nbits; i++) {
    if (MEMBER(set, i)) {
      count++;
    }
  }

  return count;
}

int _set_test(SET *set1, SET *set2) {
  int ret = _SET_EQUIV;
  int i = max(set1->nsize, set2->nsize);
  enlarge(i, set1);
  enlarge(i, set2);
  unsigned int *p1 = set1->map;
  unsigned int *p2 = set2->map;

  for (; --i >= 0; p1++, p2++) {
    if (*p1 != *p2) {
      if (*p1 & *p2) {
        return _SET_INTER;
      } else {
        ret = _SET_DISJ;
      }
    }
  }

  return ret;
}

int setcmp(SET *set1, SET *set2) 
{
  if (set1 == set2) {
    return 0;
  }
  if (set1 && !set2) {
    return 1;
  }
  if (!set1 && set2) {
    return -1;
  }

  int i, j;
  i = j = min(set1->nsize, set2->nsize);
  
  unsigned int *p1 = set1->map;
  unsigned int *p2 = set2->map;

  for (; --j >= 0; p1++, p2++) {
    if (*p1 != *p2) {
      return *p1 - *p2;
    }
  }

  if ((j = set1->nsize - i) > 0) {
    while(--j >= 0) {
      if (*p1++) {
        return 1;
      }
    }
  }

  if ((j = set2->nsize - i) > 0) {
    while(--j >= 0) {
      if (*p2++) {
        return -1;
      }
    }
  }

  return 0;
}

unsigned int sethash(SET *set) {
  unsigned int total = 0;
  int i;
  for (i = 0; i < set->nsize; i++) {
    total += set->map[i];
  }
  return total;
}

int subset(SET *set, SET *possible_subset) 
{
  int common;
  int tail;
  if (possible_subset->nsize > set->nsize) {
    common = set->nsize;
    tail = possible_subset->nsize - common;
  } else {
    common = possible_subset->nsize;
    tail = 0;
  }

  unsigned int *subsetp = possible_subset->map;
  unsigned int *setp = set->map;

  for (; --common >= 0; subsetp++, setp++) {
    if ((*subsetp & *setp) != *subsetp) {
      return 0;
    }
  }

  while (--tail >= 0) {
    if (*subsetp++) {
      return 0;
    }
  }

  return 1;
}

void _set_op(int op, SET *dest, SET *src) 
{
  int ssize = src->nsize;
  if (dest->nsize < ssize) {
    enlarge(ssize, dest);
  }

  int tail = dest->nsize - ssize;
  unsigned int *s = src->map;
  unsigned int *d = dest->map;

  switch (op) {
    case _UNION:
      while(--ssize >= 0) {
        *d++ |= *s++;
      }
      break;

    case _INTERSECT: 
      while (--ssize >= 0) {
        *d++ &= *s++;
      }
      while(--tail >= 0) {
        *d++ = 0;
      }
      break;
    
    case _DIFFERENCE:
      while (--ssize >= 0) {
        *d++ ^= *s++;
      }
      break;
    
    case _ASSIGN: 
      while(--ssize >= 0) {
        *d++ = *s++;
      }
      while(--tail >= 0) {
        *d++ = 0;
      }
      break;
  }
} 

void invert(SET *set) 
{
  unsigned int *p;
  unsigned int *end; 
  for (p = set->map, end = p + set->nsize; p < end; p++) {
    *p = ~*p;
  }
}

void truncate(SET *set) {
  if (set->map != set->defmap) {
    free(set->map);
    set->map = set->defmap;
  }
  set->nsize = _DEFSIZE;
  set->nbits = _DEFBITS;
  memset(set->map, 0, sizeof(set->defmap));
}

int next_member(SET *set) 
{
  static SET *oset = NULL;
  static int current_member = 0;
  if (!set) {
    oset = NULL;
    return 0;
  }

  if (oset != set) {
    oset = set;
    current_member = 0;
    unsigned int *map;
    for (map = set->map; current_member < set->nbits && *map == 0; ++map) {
      current_member += _BITS_IN_INT;
    }
  }

  while (current_member++ < set->nbits) {
    if (TEST(set, current_member - 1)) {
      return current_member - 1;
    }
  }

  return -1;
}

void pset(SET *set, pset_t output_routine, void *param) {
  if (!set) {
    (*output_routine)(param, "null\n", -1);
  } else {
    int i;
    int count = 0;
    next_member(NULL);
    while((i = next_member(set)) >= 0) {
      count++;
      (*output_routine)(param, "%d ", i);
    }

    next_member(NULL);
    if (!count) {
      (*output_routine)(param, "empty\n", -2);
    }
  }
}

#ifdef MAIN

int scmp(const void *a, const void *b) { return setcmp(*(SET **)a,*(SET **)b); }

int main()
{
  int i;
  SET *s1 = newset();
  SET *s2 = newset();
  SET *s3 = newset();
  SET *s4 = newset();
  SET *a[40];

  printf("adding 1024 and 2047 to s1: ");
  ADD(s1,1024);
  ADD(s1,2047);
  pset(s1, (pset_t) fprintf, stdout);
  printf("removing 1024 and 2047: ");
  REMOVE(s1, 1024);
  REMOVE(s1, 2047);
  pset(s1, (pset_t) fprintf, stdout);

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  for( i = 0; i <= 1024 ; ++i ) {
    ADD(s1, i);
    if(!TEST(s1,i) ||!MEMBER(s1,i))
      printf("initial: <%d> not in set and it should be\n", i);
  }

  for( i = 0; i <= 1024 ; ++i ) {
    if( !TEST(s1,i) || !MEMBER(s1,i) )
      printf("verify:  <%d> not in set and it should be\n", i);
  }
      
  for( i = 0; i <= 1024 ; ++i ) {
    REMOVE(s1, i);
    if( TEST(s1,i) || MEMBER(s1,i) )
    printf("initial: <%d> is in set and it shouldn't be\n", i);
  }
      
  for(i = 0; i <= 1024; ++i) {
    if( TEST(s1,i) || MEMBER(s1,i) )
      printf("verify:  <%d> is in set and it shouldn't be\n", i);
  }

  printf("add test finished: malloc set\n" );

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  truncate(s1);

  printf(IS_EQUIVALENT(s1,s2) ? "yeah!\n" : "boo\n" );

  ADD(s1, 1);
  ADD(s2, 1);
  ADD(s3, 1 );
  ADD(s1, 517);
  ADD(s2, 517);

  printf(IS_EQUIVALENT(s1, s2) ? "yeah!\n" : "boo\n");

  REMOVE(s2, 517);

  printf(!IS_EQUIVALENT(s1,s2) ? "yeah!\n" : "boo\n" );
  printf(!IS_EQUIVALENT(s2,s1) ? "yeah!\n" : "boo\n" );
  printf(!IS_EQUIVALENT(s1,s3) ? "yeah!\n" : "boo\n" );
  printf(IS_EQUIVALENT(s3,s2) ? "yeah!\n" : "boo\n" );
  printf(IS_EQUIVALENT(s2,s3) ? "yeah!\n" : "boo\n" );

  /*-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  */

  ADD(s1, 3);
  ADD(s1, 6);
  ADD(s1, 9);
  ADD(s1, 12);
  ADD(s1, 15);
  ADD(s1, 16);
  ADD(s1, 19);
  ADD(s3, 18);

  printf("s1=");      pset(s1, (pset_t) fprintf, stdout);
  printf("\ns2=");    pset(s2, (pset_t) fprintf, stdout);
  printf("\ns3=");    pset(s3, (pset_t) fprintf, stdout);
  printf("\ns4=" );   pset(s4, (pset_t) fprintf, stdout);
  printf("\n");
  printf("s1 has %d elements\n", num_ele(s1));
  printf("s2 has %d elements\n", num_ele(s2));
  printf("s3 has %d elements\n", num_ele(s3));
  printf("s4 has %d elements\n", num_ele(s4));

  s2 = dupset(s1);
  printf(IS_EQUIVALENT(s2,s1)? "dupset succeeded\n" : "dupset failed\n");

  printf("\ns1 %s empty\n", IS_EMPTY(s1) ? "IS" : "IS NOT" );
  printf("s3 %s empty\n",   IS_EMPTY(s3) ? "IS" : "IS NOT" );
  printf("s4 %s empty\n",   IS_EMPTY(s4) ? "IS" : "IS NOT" );

  printf("s1&s3 %s disjoint\n", IS_DISJOINT    (s1,s3) ? "ARE":"ARE NOT");
  printf("s1&s4 %s disjoint\n", IS_DISJOINT    (s1,s4) ? "ARE":"ARE NOT");

  printf("s1&s3 %s intersect\n",IS_INTERSECTING(s1,s3) ? "DO" : "DO NOT");
  printf("s1&s4 %s intersect\n",IS_INTERSECTING(s1,s4) ? "DO" : "DO NOT");

  printf("s1 %s a subset of s1\n", subset(s1,s1) ? "IS" : "IS NOT" );
  printf("s3 %s a subset of s3\n", subset(s3,s3) ? "IS" : "IS NOT" );
  printf("s4 %s a subset of s4\n", subset(s4,s4) ? "IS" : "IS NOT" );
  printf("s1 %s a subset of s3\n", subset(s3,s1) ? "IS" : "IS NOT" );
  printf("s1 %s a subset of s4\n", subset(s4,s1) ? "IS" : "IS NOT" );
  printf("s3 %s a subset of s1\n", subset(s1,s3) ? "IS" : "IS NOT" );
  printf("s3 %s a subset of s4\n", subset(s4,s3) ? "IS" : "IS NOT" );
  printf("s4 %s a subset of s1\n", subset(s1,s4) ? "IS" : "IS NOT" );
  printf("s4 %s a subset of s3\n", subset(s3,s4) ? "IS" : "IS NOT" );

  printf("\nAdding 18 to s1:\n");
  ADD(s1, 18);
  printf("s1 %s a subset of s1\n", subset(s1,s1) ? "IS" : "IS NOT" );
  printf("s3 %s a subset of s3\n", subset(s3,s3) ? "IS" : "IS NOT" );
  printf("s1 %s a subset of s3\n", subset(s3,s1) ? "IS" : "IS NOT" );
  printf("s3 %s a subset of s1\n", subset(s1,s3) ? "IS" : "IS NOT" );

  ASSIGN(s2,s3); puts("\ns3       =");
  pset(s2, (pset_t) fprintf, stdout );
  ASSIGN(s2,s3); UNION(s2,s1); puts("\ns1 UNION s3=");
  pset(s2, (pset_t) fprintf, stdout );
  ASSIGN(s2,s3); INTERSECT(s2,s1); puts("\ns1 INTER s3=");
  pset(s2, (pset_t) fprintf, stdout );
  ASSIGN(s2,s3); DIFFERENCE(s2,s1); puts("\ns1 DIFF s3=");
  pset(s2, (pset_t) fprintf, stdout );

  truncate(s2);
  printf("s2 has%s been emptied\n", IS_EMPTY(s2) ? "" : " NOT" );

  invert(s2); printf("\ns2 inverted = "); pset(s2,(pset_t) fprintf,stdout);
  CLEAR (s2); printf("\ns2 cleared  = "); pset(s2,(pset_t) fprintf,stdout);
  FILL  (s2); printf("\ns2 filled   = "); pset(s2,(pset_t) fprintf,stdout);

  printf("\ns1="); pset(s1, (pset_t) fprintf, stdout);
  printf("\ns3="); pset(s3, (pset_t) fprintf, stdout);
  printf("\ns4="); pset(s4, (pset_t) fprintf, stdout);
  printf("\n");

  /* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */

  for(i = 40; --i >= 0;) {
    a[i] = newset();
    ADD(a[i], i % 20);
  }

  ADD(a[0],  418); REMOVE(a[0],  418);
  ADD(a[10], 418); REMOVE(a[10], 418);

  printf("\nunsorted:\n");
  for(i = 0; i < 40; i++) {
    printf("Set %d: ", i);
    pset(a[i], (pset_t) fprintf, stdout);
    printf(i & 1 ? "\n" : "\t");
  }

  qsort(a, 40, sizeof(a[0]), scmp);

  printf("\nsorted:\n");
  for(i = 0; i < 40; i++) {
    printf("Set %d: ", i);
    pset(a[i], (pset_t) fprintf, stdout);
    printf( i & 1 ? "\n" : "\t");
  }
  return 0;
}
#endif






  