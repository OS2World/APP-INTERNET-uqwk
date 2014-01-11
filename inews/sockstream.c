/* !CRF! - sockstream.c */
/*
    In OS/2, socket discriptors are unrelated to file descriptors.  This
    makes it impossible to use fdopen to open a socket.  These funtions
    go around this limitation by bypassing the fxxx() functions such as
    fgets, fprintf, etc..

        Claudio Fahey (claudio@uclink.berkeley.edu)
*/

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "sockstream.h"

#ifdef BUFFERSOCKGETS

char *sockgets( char *string, int n, int s )
    {
    static char buffer1[BUFSIZE], *buffer = buffer1;
    static int bufsize = 0;

    char *lfptr;
    int stringpos = 0, copysize, flReturnString = 0;

    for (;;)
        {
        /* get position of LF in the buffer */
        lfptr = memchr( buffer, '\n', bufsize );

        /* if there is a LF in the buffer, then ... */
        if ( lfptr )
            {
            copysize = lfptr - buffer + 1;
            flReturnString = 1;
            }
        else
            copysize = bufsize;

        /* make sure we won't write more than n-1 characters */
        if ( copysize > (n - 1) - stringpos )
            {
            copysize = (n - 1) - stringpos;
            flReturnString = 1;
            }

        /* copy copysize characters from buffer into the string */
        memcpy( string + stringpos, buffer, copysize );
        stringpos += copysize;

        /* if we got the whole string, then set buffer to the unused data
           and return the string */
        if ( flReturnString )
            {
            string[stringpos] = '\0';
            bufsize -= copysize;
            buffer += copysize;
            return string;
            }

        /* reset buffer and receive more data into buffer */
        buffer = buffer1;
        if ( (bufsize = recv( s, buffer, BUFSIZE, 0 )) == -1 )
            {
            perror( "Error on socket recv" );
            return NULL;
            }
        }
    }

#else /* !BUFFERSOCKGETS */

int sockgetc( int s )
    {
    char c;

    if ( recv( s, &c, 1, 0 ) == -1 )
        {
        perror( "Error on socket recv" );
        return EOF;
        }
    return c;
    }

char *sockgets( char *string, int n, int s )
    {
    int rc, stringpos;

    for ( stringpos = 0 ; stringpos < n-1 ; stringpos++  )
        {
        rc = recv( s, string + stringpos, 1, 0 );
        if ( string[stringpos] == '\n' )
            {
            stringpos++;
            break;
            }
        if ( rc == 0 )
            break;
        if ( rc == -1 )
            {
            perror( "Error on socket recv" );
            return NULL;
            }
        }
    string[stringpos] = '\0';
    return string;
    }

#endif /* BUFFERSOCKGETS */

int sockputc( int c, int s )
    {
    if ( send( s, (char*) &c, 1, 0 ) == -1 )
        {
        perror( "Error on socket send" );
        return EOF;
        }
    return c;
    }

int sockputs( char *string, int s )
    {
    if ( send( s, string, strlen(string), 0 ) == -1 )
        {
        perror( "Error on socket send" );
        return -1;
        }
    else
        return 1;
    }

int sockprintf( int s, char *format, ... )
    {
    static char buffer[BUFSIZE];
    va_list arg_ptr;
    int retval;
 
    va_start( arg_ptr, format );

    retval = vsprintf( buffer, format, arg_ptr );
    if ( send( s, buffer, strlen(buffer), 0 ) == -1 )
        {
        perror( "Error on socket send" );
        return EOF;
        }
 
    va_end( arg_ptr );
    return retval;
    }

