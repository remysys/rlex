#ifdef _INPUT_H

void ii_io(int (*open_funct)(char *, int), int (*close_funct)(int), int (*read_funct)(int, void *, unsigned int)); 
unsigned char *ii_text();
int ii_length();
int ii_lineno();

unsigned char *ii_ptext();
int ii_plength();
int ii_plineno();

unsigned char *ii_mark_start();
unsigned char *ii_mark_end();
unsigned char *ii_move_start();
unsigned char *ii_to_mark();
unsigned char *ii_mark_prev();

int ii_advance();
int ii_flush(int force);
int ii_fillbuf(unsigned char *starting_at);
int ii_look(int n);
int ii_pushback(int n);

void ii_term();
void ii_unterm();
void ii_input();
void ii_unput(int c);

int ii_lookahead(int n);
int ii_flushbuf();

#endif


