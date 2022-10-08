INCLUDE = ../include
CC = gcc
CFLAGS = -I$(INCLUDE) -g

#CFLAGS = -I$(INCLUDE) -DMAIN

.PHONY: all clean test install uninstall
all: rlex

OBJS = lex.o squash.o printnfa.o print.o nfa.o minimize.o \
	egrep.o dfa.o 

lib:
	make -C ../lib

rlex: $(OBJS) lib
	gcc -o $@ $(OBJS) ../lib/libcomp.a


install:
	sudo cp -fr rlex /usr/local/bin/
	sudo cp -fr ../lib/liblex.a /usr/local/lib64/
	sudo cp -fr lex.par /usr/local/lib64/
	sudo mkdir -p /usr/local/include/rlex
	sudo cp -fr ../include/l.h /usr/local/include/rlex/

uninstall:
	rm -fr /usr/local/bin/rlex
	rm -fr /usr/local/lib64/lex.par
	rm -fr /usr/local/lib64/liblex.a
	rm -fr /usr/local/include/rlex
  
#generate word count program

test:
	rlex ../test/wc.lex
	gcc -o wc yylex.c -llex

clean:
	rm -fr *.o
	rm -fr rlex
	rm -fr yylex.c
	rm -fr wc
	make -C ../lib clean

lex.o:      lex.c
squash.o:   squash.c
printnfa.o: printnfa.c
print.o:    print.c
nfa.o:      nfa.c
minimize.o: minimize.c
egrep.o:    egrep.c
dfa.o:      dfa.c
