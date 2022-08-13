#ifndef _COMM_H

#define max(a,b) ( ((a) > (b)) ? (a) : (b))
#define min(a,b) ( ((a) < (b)) ? (a) : (b))

#define NUMELE(a)	(sizeof(a)/sizeof(*(a)))
#define LASTELE(a)	((a) + (NUMELE(a)-1))
#define TOOHIGH(a, p)	((p) - (a) > (NUMELE(a) - 1))
#define TOOLOW(a, p)	((p) - (a) <  0 )
#define INBOUNDS(a, p)	(!(TOOHIGH(a,p) || TOOLOW(a,p)))

#endif