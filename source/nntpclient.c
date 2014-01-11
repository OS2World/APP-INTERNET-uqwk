
/*
 * 
 * This software is Copyright 1991 by Stan Barber. 
 * 
 * Permission is hereby granted to copy, reproduce, redistribute or otherwise
 * use this software as long as: there is no monetary profit gained
 * specifically from the use or reproduction of this software, it is not
 * sold, rented, traded or otherwise marketed, and this copyright notice is
 * included prominently in any copy made. 
 *
 * The author make no claims as to the fitness or correctness of this software
 * for any use whatsoever, and it is provided as is. Any use of this software
 *is at the user's own risk. 
 */


/*
 * The functions in this file come from rn-1.4.4
 * They have been modified to work with uqwk.
 * They are used to read news articles from a remote
 * news server.
 *
 *  -- Ken Whedbee   3/31/93
 *
 */

#include <stdio.h>
#include "nntp.h"
#include "uqwk.h"

#define Nullfp Null(FILE *)
#define MAXFILENAME 128
#define SERVER_FILE "/local/lib/news/server"	/* news server file */


char ser_line[NNTP_STRLEN];
int openart;

extern  char    *getserverbyfile ();
extern  int     server_init ();
extern  void    put_server ();
extern  int     get_server ();
extern  void    close_server ();

void connect_nntp();
FILE *getactive_nntp(); 
void group_nntp();
FILE *nntpopen();
int nntp_get();


void
connect_nntp()
{
    char *server;
    int response;

    /* open connection to server if appropriate */

    server = getserverbyfile(SERVER_FILE);
    if (server == NULL) {
	fprintf(stderr, "Can't get the name of the news server from %s\n",
		SERVER_FILE);
	fprintf(stderr,
	  "Either fix this file, or put NNTPSERVER in your environment.\n");
	exit(1);
    }

    response = server_init(server);
    if (response < 0) {
	fprintf(stderr,
	    "Couldn't connect to %s news server, try again later.\n",
		server);
	exit(1);
    }

    if (handle_server_response(response, server) < 0)
	exit(1);

    /* This is for INN */
    put_server ("mode reader");
    nntp_get (ser_line, sizeof(ser_line));

}



/* open active file, etc. */

FILE
*getactive_nntp()
{
    char active_name[MAXFILENAME];
    register FILE *actfp;

    /* open the active file */

    put_server("LIST");		/* tell server we want the active file */
    nntp_get(ser_line, sizeof(ser_line));
    if (*ser_line != CHAR_OK) {		/* and then see if that's ok */
	fprintf(stdout, "Can't get active file from server: \n%s\n", ser_line);
	exit(1);
    }
                                        /* make a temporary name */
    sprintf(active_name,"%s/rrnact.%d",TEMPDIR,getpid());

    actfp = fopen(active_name, "w+b");	/* and get ready */

    if (actfp == (FILE *)NULL) {
	printf("Cant open %s\n",active_name), fflush(stdout);
	exit(1);
    }

    while (1) {
	if (nntp_get(ser_line, sizeof(ser_line)) < 0) {
	    printf("Can't get active file from server\n");
	    exit(1);
	}
	if (ser_line[0] == '.')		/* while there's another line */
		break;			/* get it and write it to */
	fputs(ser_line, actfp);
	putc('\n', actfp);
    }

    if (ferror(actfp)) {
	printf("Error writing to active file %s.\n", active_name), fflush(stdout);
	exit(1);
    }
    if (fseek(actfp,0L,0) == -1) {	/* just get to the beginning */
	printf("Error seeking in active file.\n"), fflush(stdout);
	exit(1);
    }

    return actfp; 			/* return active file ptr */
}


void
group_nntp(ngname)    /* select newsgroup to read from */
char *ngname;
{

    sprintf(ser_line, "GROUP %s", ngname);
    put_server(ser_line);
    if (nntp_get(ser_line, sizeof(ser_line)) < 0) {
	fprintf(stderr, "\nrrn: Unexpected close of server socket.\n");
	exit(1);
    }
    if (*ser_line != CHAR_OK) {
	if (atoi(ser_line) != ERR_NOGROUP){
		fprintf(stderr, "\nrrn: server response to GROUP %s:\n%s\n",
			ngname, ser_line);
		exit(1);
	}
    }

}




/**
 **  example usage:
 **
 **  artfp = nntpopen(artnum,GET_ARTICLE);
 **
 **/

FILE 
*nntpopen(artnum,function)
int artnum;
int function;
{
    char artname[MAXFILENAME];		/* filename of current article */
    FILE *artfp;

    if (artnum < 1)
	return (FILE *)NULL;

    sprintf(artname,"%s/rrn%ld.%ld", TEMPDIR, (long) artnum, getpid());
    artfp = fopen(artname, "w+b");	/* create the temporary article */
    if (artfp == (FILE *)NULL) {
	unlink(artname);
	return (FILE *)NULL;
    }
    switch (function){
	    case GET_STATUS:
		function = GET_HEADER;	/* fall through */
	    case GET_HEADER:
		sprintf(ser_line, "HEAD %ld", (long)artnum);
		break;
	    case GET_ARTICLE:
		sprintf(ser_line, "ARTICLE %ld", (long)artnum);
		break;
    }	    
    put_server(ser_line);		/* ask the server for the article */
    if (nntp_get(ser_line, sizeof(ser_line)) < 0) {
	fprintf(stderr, "\nrrn: Unexpected close of server socket.\n");
	exit(1);
    }
    if (*ser_line == CHAR_FATAL) {	/* Fatal error */
		fprintf(stderr,"\nrrn: %s\n",ser_line);
		exit(1);
	}
    if (*ser_line != CHAR_OK) {		/* and get it's reaction */
	fclose(artfp);
	artfp = (FILE *)NULL;
	unlink(artname);
        return (FILE *)NULL;
    }
    for (;;) {
        if (nntp_get(ser_line, sizeof(ser_line)) < 0) {
	    fprintf(stderr, "\nrrn: Unexpected close of server socket.\n");
	    exit(1);
	}
	if (ser_line[0] == '.' && ser_line[1] == '\0')
		break;
	fputs((ser_line[0] == '.' ? ser_line + 1 : ser_line), artfp);
	putc('\n', artfp);
    }
    if (function == GET_HEADER)
	 putc('\n', artfp); /* req'd blank line after header */
    fseek(artfp, 0L, 0);		/* Then get back to the start */
    openart = artnum;
/*  printf("got article %ld\n",(long) artnum); */

    return artfp;			/* and return either fp or NULL */
}




int 
nntp_get(buf, len)
char *buf;
int  len;
{
 	int n;

 	n = get_server(buf, len);

 	return n;
}
