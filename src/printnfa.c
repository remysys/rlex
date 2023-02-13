#include <stdio.h>
#include <ctype.h>
#include <compiler.h>
#include <set.h>
#include "nfa.h"

/*--------------------------------------------------------------
 *  printnfa.c routine to print out a NFA structure in human-readable form.
 */

void printccl(SET *set) 
{
	int i;
	putchar('[');
	for (i = 0; i <= 0x7f; i++) {
		if (TEST(set, i)) {
			if (i < ' ') {
				printf("^%c", i + '@');
			} else {
				printf("%c", i);
			}
		}
	}
	putchar(']');
}

char *plab (NFA *nfa, NFA *state)
{
	/* return a pointer to a buffer containing the state number. The buffer is
     * overwritten on each call so don't put more than one plab() call in an
     * argument to printf().
     */
	
	static char buf[32];
	if (!nfa || !state) {
		return "--";
	}

	sprintf(buf, "%2ld", (long)(state - nfa));
	return buf;
}

void print_nfa(NFA * nfa, int len, NFA *start)
{
	NFA *s = nfa;
	printf("\n----------------- NFA ---------------\n");
	for (; --len >= 0; nfa++) {
		printf("NFA state %s: ", plab(s, nfa));
		if (!nfa->next) {
			printf("(TERMINAL)");
		} else {
			printf("--> %s ",  plab(s, nfa->next));
			printf("(%s) on ", plab(s, nfa->next2));

			switch (nfa->edge) {
				case CCL: printccl(nfa->bitset); break;
				case EPSILON: printf  ("EPSILON ");	break;
				default: putchar(nfa->edge);	break;
			}
		}

		if (nfa == start) {
			printf(" (START STATE)");
		}

		if(nfa->accept) {
			printf(" accepting %s<%s>%s", 
									nfa->anchor & START ? "^" : "",
									nfa->accept,
									nfa->anchor & END   ? "$" : "");
		}

		printf("\n");	
	}
}
