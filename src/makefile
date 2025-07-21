INCLUDE = ../include
CC = gcc
CFLAGS = -I$(INCLUDE) -g

.PHONY: all clean test install uninstall lib


all: rlex

OBJS = lex.o squash.o printnfa.o print.o nfa.o minimize.o egrep.o dfa.o


rlex: $(OBJS) lib
	$(CC) $(CFLAGS) -o $@ $(OBJS) ../lib/libcomp.a

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

lib:
	$(MAKE) -C ../lib

test: wc
	@echo "running wc test (word count on lex.c)"
	./wc lex.c


wc: lex.yy.c rlex
	@echo "compiling lex.yy.c into executable wc"
	$(CC) $(CFLAGS) -o $@ $< ../lib/libl.a


lex.yy.c: ../test/wc.lex rlex
	@echo "generating lex.yy.c from wc.lex"
	./rlex $< > $@

install:
	cp -fr rlex /usr/local/bin/
	cp -fr lex.par /usr/local/lib/
	cp -fr ../lib/libl.a /usr/local/lib/
	cp -fr ../include/l.h /usr/local/include/

uninstall:
	rm -f /usr/local/bin/rlex
	rm -f /usr/local/lib/lex.par
	rm -f /usr/local/lib/libl.a
	rm -f /usr/local/include/l.h


clean:
	rm -f *.o rlex wc lex.yy.c
	$(MAKE) -C ../lib clean
