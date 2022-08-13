INCLUDE	= /root/ctools/
CFLAGS  = -I$(INCLUDE) -g
#CFLAGS  = -I$(INCLUDE) -DMAIN

OBJ = nfa.o set.o hash.o sec.o printnfa.o

nfa.o: nfa.c
	gcc $(CFLAGS) -c $^ -o $@
printnfa.o:	printnfa.c
	gcc $(CFLAGS) -c $^ -o $@
set.o: set.c 
	gcc $(CFLAGS) -c $^ -o $@
hash.o: hash.c
	gcc $(CFLAGS) -c $^ -o $@
esc.o: esc.c
	gcc $(CFLAGS) -c $^ -o $@
egrep.o: egrep.c
	gcc $(CFLAGS) -c $^ -o $@


test: hash nfa set egrep 

hash: hash.o
	gcc -o $@ $^
	
set: set.o
	gcc -o $@ $^

nfa: nfa.o set.o hash.o printnfa.o esc.o
	gcc -o $@ $^

egrep: egrep.o nfa.o set.o hash.o printnfa.o esc.o
	gcc -o $@ $^

clean:
	rm -fr *.o
	rm -fr hash
	rm -fr nfa
	rm -fr set
	rm -fr egrep
