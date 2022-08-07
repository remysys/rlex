INCLUDE	= /root/ctools/
CFLAGS  = -I$(INCLUDE) -DMAIN -g

all: test_set test_hash
test_set: set.c
	gcc $(CFLAGS) -o $@ $^

test_hash: hash.c
	gcc $(CFLAGS) -o $@ $^

clean:
	rm -fr test_hash
	rm -fr test_set
