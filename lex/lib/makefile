INCLUDE  = /root/ctools/lex/include
LIBLEX   = /root/ctools/lex/lib/liblex.a
LIBCOMP	 = /root/ctools/lex/lib/libcomp.a

CC = gcc
CFLAGS  = -I$(INCLUDE)

LEX_OBJ	 = input.o ferr.o l.o
COMP_OBJ = hash.o set.o escape.o printutils.o ferr.o
#----------------------------------------------------------------------
all: $(LIBLEX) $(LIBCOMP)

$(LIBLEX):	$(LEX_OBJ)
	rm -fr $(LIBLEX)
	ar rvs $(LIBLEX) $(LEX_OBJ)

$(LIBCOMP):	$(COMP_OBJ)
	rm -fr $(LIBCOMP)
	ar rvs $(LIBCOMP) $(COMP_OBJ)

# the objects that follow are the lex run-time library

input.o:	input.c
ferr.o:		ferr.c
l.o: 			l.c

# the following routines are used by lex, but not by the
# programs that lex generate

hash.o: 			hash.c
set.o: 				set.c
escape.o: 		escape.c
printutils.o: printutils.c
ferr.o:				ferr.c

clean:
	rm -fr $(LEX_OBJ) $(LIBLEX)
	rm -fr $(COMP_OBJ) $(LIBCOMP)