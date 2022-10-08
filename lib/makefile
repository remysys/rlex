INCLUDE = ../include
LIBLEX  = liblex.a
LIBCOMP = libcomp.a

CC = gcc
CFLAGS = -I$(INCLUDE)

.PHONY: all clean

LEX_OBJ  = input.o ferr.o yymain.o yywrap.o yyinitlex.o
COMP_OBJ = hash.o set.o escape.o printutils.o ferr.o

all: $(LIBLEX) $(LIBCOMP)

$(LIBLEX): $(LEX_OBJ)
	rm -fr $(LIBLEX)
	ar rvs $(LIBLEX) $(LEX_OBJ)

$(LIBCOMP): $(COMP_OBJ)
	rm -fr $(LIBCOMP)
	ar rvs $(LIBCOMP) $(COMP_OBJ)

# the objects that follow are the lex run-time library

input.o:     input.c
ferr.o:      ferr.c
yymain.o:    yymain.c
yywrap.o:    yywrap.c
yyinitlex.o: yyinitlex.c

# the following routines are used by lex, but not by the
# programs that lex generate

hash.o:       hash.c
set.o:        set.c
escape.o:     escape.c
printutils.o: printutils.c
ferr.o:       ferr.c

clean:
	rm -fr $(LEX_OBJ) $(LIBLEX)
	rm -fr $(COMP_OBJ) $(LIBCOMP)