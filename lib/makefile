INCLUDE = ../include
LIBL  = libl.a
LIBCOMP = libcomp.a

CC = gcc
CFLAGS = -I$(INCLUDE)

.PHONY: all clean

L_OBJ  = ferr.o input.o yymain.o yywrap.o yyinitlex.o yyinitox.o
COMP_OBJ = hash.o set.o escape.o printutils.o ferr.o fputstr.o printv.o driver.o

all: $(LIBL) $(LIBCOMP)

$(LIBL): $(L_OBJ)
	rm -fr $(LIBL)
	ar rvs $(LIBL) $(L_OBJ)

$(LIBCOMP): $(COMP_OBJ)
	rm -fr $(LIBCOMP)
	ar rvs $(LIBCOMP) $(COMP_OBJ)

# the objects that follow are the lex run-time library

input.o:     input.c
yymain.o:    yymain.c
yywrap.o:    yywrap.c
yyinitlex.o: yyinitlex.c
yyinitox.o:  yyinitox.c

# the following routines are used by lex, but not by the
# programs that lex generate

hash.o:       hash.c
set.o:        set.c
escape.o:     escape.c
printutils.o: printutils.c
ferr.o:       ferr.c
fputstr.o:    fputstr.c
printv.o:     printv.c
driver.o:     driver.c

clean:
	rm -fr $(L_OBJ) $(LIBL)
	rm -fr $(COMP_OBJ) $(LIBCOMP)