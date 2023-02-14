INCLUDE = ../include
CC = gcc
CFLAGS = -I$(INCLUDE) -g

#CFLAGS = -I$(INCLUDE) -DMAIN

.PHONY: all clean test install uninstall
all: rlex

OBJS = lex.o squash.o printnfa.o print.o nfa.o minimize.o egrep.o dfa.o 

lib:
	make -C ../lib

rlex: $(OBJS) lib
	gcc -o $@ $(OBJS) ../lib/libcomp.a


install:
	sudo cp -fr rlex /usr/local/bin/
	sudo cp -fr lex.par /usr/local/lib64/
	sudo cp -fr ../lib/libl.a /usr/local/lib64/
	sudo cp -fr ../include/l.h /usr/local/include/

uninstall:
	sudo rm -fr /usr/local/bin/rlex
	sudo rm -fr /usr/local/lib64/lex.par
	sudo rm -fr /usr/local/lib64/libl.a
	sudo rm -fr /usr/local/include/l.h
  
#generate word count program

test:
	rlex ../test/wc.lex
	gcc -o wc lex.yy.c -ll

clean:
	rm -fr *.o
	rm -fr rlex
	rm -fr lex.yy.c
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
