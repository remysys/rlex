INCLUDE	= ../include
CC = gcc
CFLAGS  = -I$(INCLUDE) -g

#CFLAGS  = -I$(INCLUDE) -DMAIN

OBJS = lex.o squash.o printnfa.o print.o nfa.o minimize.o \
		egrep.o dfa.o 

lib: 
	make -C ../lib

rlex:  $(OBJS) lib
	gcc -o $@ $(OBJS) ../lib/libcomp.a

#generate word count program

test: rlex
	./rlex -v ../test/wc.lex
	gcc $(CFLAGS) -c yylex.c
	gcc -o wc yylex.o ../lib/liblex.a

clean:
	rm -fr *.o
	rm -fr rlex
	rm -fr yylex.c
	rm -fr wc
	make -C ../lib clean

lex.o: 			lex.c
squash.o:		squash.c
printnfa.o: printnfa.c
print.o: 		print.c
nfa.o: 			nfa.c
minimize.o: minimize.c
egrep.o: 		egrep.c
dfa.o: 			dfa.c
