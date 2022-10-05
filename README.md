rlex: a clear version of the standard UNIX utility lex
===
rlex is a tool program for generating lexical analyzers (scanners). It supports the lexical rules of the c language

![rlex internals](doc/rlex.png)

Usage
-----------
```
$ cd src && make && make install
$ rlex 
rlex 0.01 [gcc 4.8.5] [Aug 28 2022]. (c) 2022, ****. all rights reserved.
rlex: file name required

usage is: rlex [options] file
-f  for (f)ast. don't compress tables
-h  suppress (h)eader comment that describes state machine
-l  suppress #(l)ine directives in the output
-t  send output to standard output instead of yylex.c
-v  (v)erbose mode, print statistics
-V  more (V)erbose, print internal diagnostics as rlex runs
```

generate a word count program that, like the utility wc

```
$ rlex ../test/wc.lex
$ gcc -o wc yylex.c -llex
$ ./wc $filename
```

or run 'make test' to generate the wc test program
```
$ make test
```
