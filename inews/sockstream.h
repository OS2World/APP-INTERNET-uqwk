/* !CRF! - sockstream.h */

/* #define BUFFERSOCKGETS */
#define BUFSIZE 4096    /* buffer size used by sockgets and sockprintf */

int sockgetc( int s );  /* only available when BUFFERSOCKGETS is undefined */
char *sockgets( char *string, int n, int s );
int sockputc( int c, int s );
int sockputs( char *string, int s );
int sockprintf( int s, char *format, ... );

