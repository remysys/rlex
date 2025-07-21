INCLUDE = ../include
CC = gcc
CFLAGS = -I$(INCLUDE) -g

#CFLAGS = -I$(INCLUDE) -DMAIN

.PHONY: all clean test
all: rlex

OBJS = lex.o squash.o printnfa.o print.o nfa.o minimize.o egrep.o dfa.o 

lib:
	make -C ../lib

rlex: $(OBJS) lib
	gcc -o $@ $(OBJS) ../lib/libcomp.a
  
#generate word count program

test:
	@echo "running wc (short for word count) test"
	@echo "generating lex.yy.c from wc.lex"
	./rlex ../test/wc.lex
	@echo "compiling lex.yy.c into executable wc"
	gcc -o wc lex.yy.c ../lib/libl.a
	@echo "running wc on lex.c"
	./wc lex.c

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
