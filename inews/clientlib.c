#ifndef lint
static char	*rcsid = "@(#)$Header: clientlib.c,v 1.16 91/01/12 15:12:37 sob Exp $";
#endif

/*
 * This software is Copyright 1991 by Stan Barber. 
 *
 * Permission is hereby granted to copy, reproduce, redistribute or otherwise
 * use this software as long as: there is no monetary profit gained
 * specifically from the use or reproduction or this software, it is not
 * sold, rented, traded or otherwise marketed, and this copyright notice is
 * included prominently in any copy made. 
 *
 * The author make no claims as to the fitness or correctness of this software
 * for any use whatsoever, and it is provided as is. Any use of this software
 * is at the user's own risk. 
 *
 */
/*
 * NNTP client routines.
 */

/*
 * The functions in this file come from NNTP 1.5.10
 * They have been modified slightly to work with uqwk.
 * They are used to read news articles from a remote
 * news server.
 *
 *  -- Ken Whedbee   3/31/93
 *
 */


#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#ifdef TLI
#include	<fcntl.h>
#include	<tiuser.h>
#include	<stropts.h>
#include	<sys/socket.h>
#ifdef WIN_TCP
#include	<sys/in.h>
#else
#include	<netinet/in.h>
#endif
# define	IPPORT_NNTP	((unsigned short) 119)
# include 	<netdb.h>	/* All TLI implementations may not have this */
#else /* !TLI */
#include <sys/socket.h>
#include <netinet/in.h>
#ifndef EXCELAN
# include <netdb.h>
#endif /* !EXCELAN */
#endif /* !TLI */

#ifdef USG
# define	index	strchr
# define        bcopy(a,b,c)   memcpy(b,a,c)
# define        bzero(a,b)     memset(a,'\0',b)
#endif /* USG */

#ifdef EXCELAN
# define	IPPORT_NNTP	((unsigned short) 119)
#if __STDC__
int connect();
unsigned short htons();
unsigned long rhost();
int rresvport();
int socket();
#endif
#endif

#ifdef DECNET
#include <netdnet/dn.h>
#include <netdnet/dnetdb.h>
#endif /* DECNET */

#include "nntp.h"

#ifdef NOFDOPEN
 #include "sockstream.h"
 int     sockt = -1;
#else
 FILE	*ser_rd_fp = NULL;
 FILE	*ser_wr_fp = NULL;
#endif

/*
 * getserverbyfile	Get the name of a server from a named file.
 *			Handle white space and comments.
 *			Use NNTPSERVER environment variable if set.
 *
 *	Parameters:	"file" is the name of the file to read.
 *
 *	Returns:	Pointer to static data area containing the
 *			first non-ws/comment line in the file.
 *			NULL on error (or lack of entry in file).
 *
 *	Side effects:	None.
 */

char *
getserverbyfile(file)
char	*file;
{
	register FILE	*fp;
	register char	*cp;
	static char	buf[256];
	char		*index();
	char		*getenv();
	char		*strcpy();

	if (cp = getenv("NNTPSERVER")) {
		(void) strcpy(buf, cp);
		return (buf);
	}

	if (file == NULL)
		return (NULL);

	fp = fopen(file, "rb");
	if (fp == NULL)
		return (NULL);

	while (fgets(buf, sizeof (buf), fp) != NULL) {
		if (*buf == '\n' || *buf == '#')
			continue;
		cp = index(buf, '\n');
		if (cp)
			*cp = '\0';
		(void) fclose(fp);
		return (buf);
	}

	(void) fclose(fp);
	return (NULL);			 /* No entry */
}


/*
 * server_init  Get a connection to the remote news server.
 *
 *	Parameters:	"machine" is the machine to connect to.
 *
 *	Returns:	-1 on error
 *			server's initial response code on success.
 *
 *	Side effects:	Connects to server.
 *			"ser_rd_fp" and "ser_wr_fp" are fp's
 *			for reading and writing to server.
 */

server_init(machine)
char	*machine;
{
	int	sockt_rd, sockt_wr;
	char	line[256];
	char	*index();
	char	*cp;

#ifdef NOFDOPEN
    sockt = get_tcp_socket(machine);
    if (sockt < 0)
        return (-1);
#else /* !NOFDOPEN */
#ifdef DECNET
	cp = index(machine, ':');

	if (cp && cp[1] == ':') {
		*cp = '\0';
		sockt_rd = get_dnet_socket(machine);
	} else
		sockt_rd = get_tcp_socket(machine);
#else
	sockt_rd = get_tcp_socket(machine);
#endif

	if (sockt_rd < 0)
		return (-1);

	/*
	 * Now we'll make file pointers (i.e., buffered I/O) out of
	 * the socket file descriptor.  Note that we can't just
	 * open a fp for reading and writing -- we have to open
	 * up two separate fp's, one for reading, one for writing.
	 */

	if ((ser_rd_fp = fdopen(sockt_rd, "r")) == NULL) {
		perror("server_init: fdopen #1");
		return (-1);
	}

	sockt_wr = dup(sockt_rd);
#ifdef TLI
	if (t_sync(sockt_rd) < 0){	/* Sync up new fd with TLI */
    		t_error("server_init: t_sync");
		ser_rd_fp = NULL;		/* from above */
		return (-1);
	}
#endif
	if ((ser_wr_fp = fdopen(sockt_wr, "w")) == NULL) {
		perror("server_init: fdopen #2");
		ser_rd_fp = NULL;		/* from above */
		return (-1);
	}
#endif /* NOFDOPEN */

	/* Now get the server's signon message */

	(void) get_server(line, sizeof(line));
	return (atoi(line));
}


/*
 * get_tcp_socket -- get us a socket connected to the news server.
 *
 *	Parameters:	"machine" is the machine the server is running on.
 *
 *	Returns:	Socket connected to the news server if
 *			all is ok, else -1 on error.
 *
 *	Side effects:	Connects to server.
 *
 *	Errors:		Printed via perror.
 */

get_tcp_socket(machine)
char	*machine;	/* remote host */
{
	int	s;
	struct	sockaddr_in sin;
#ifdef TLI 
	char 	*t_alloc();
	struct	t_call	*callptr;
	/*
	 * Create a TCP transport endpoint.
	 */
	if ((s = t_open("/dev/tcp", O_RDWR, (struct t_info*) 0)) < 0){
		t_error("t_open: can't t_open /dev/tcp");
		return(-1);
	}
	if(t_bind(s, (struct t_bind *)0, (struct t_bind *)0) < 0){
	    	t_error("t_bind");
		t_close(s);
		return(-1);
	}
	bzero((char *) &sin, sizeof(sin));	
	sin.sin_family = AF_INET;
	sin.sin_port = htons(IPPORT_NNTP);
	if (!isdigit(*machine) ||
	    (long)(sin.sin_addr.s_addr = inet_addr(machine)) == -1){
		struct	hostent *gethostbyname(), *hp;
		if((hp = gethostbyname(machine)) == NULL){
		    	fprintf(stderr,"gethostbyname: %s: host unknown\n",
				machine);
			t_close(s);
			return(-1);
		}
		bcopy(hp->h_addr, (char *) &sin.sin_addr, hp->h_length);
	}
	/*
	 * Allocate a t_call structure and initialize it.
	 * Let t_alloc() initialize the addr structure of the t_call structure.
	 */
	if ((callptr = (struct t_call *) t_alloc(s,T_CALL,T_ADDR)) == NULL){
		t_error("t_alloc");
		t_close(s);
		return(-1);
	}

	callptr->addr.maxlen = sizeof(sin);
	callptr->addr.len = sizeof(sin);
	callptr->addr.buf = (char *) &sin;
	callptr->opt.len = 0;			/* no options */
	callptr->udata.len = 0;			/* no user data with connect */

	/*
	 * Connect to the server.
	 */
	if (t_connect(s, callptr, (struct t_call *) 0) < 0) {
		t_error("t_connect");
		t_close(s);
		return(-1);
	}

	/*
	 * Now replace the timod module with the tirdwr module so that
	 * standard read() and write() system calls can be used on the
	 * descriptor.
	 */

	if (ioctl(s,  I_POP,  (char *) 0) < 0){
		perror("I_POP(timod)");
		t_close(s);
		return(-1);
	}

	if (ioctl(s,  I_PUSH, "tirdwr") < 0){
		perror("I_PUSH(tirdwr)");
		t_close(s);
		return(-1);
	}
	
#else /* !TLI */
#ifndef EXCELAN
	struct	servent *getservbyname(), *sp;
	struct	hostent *gethostbyname(), *hp;
#ifdef h_addr
	int	x = 0;
	register char **cp;
	static char *alist[1];
#endif /* h_addr */
	unsigned long inet_addr();
	static struct hostent def;
	static struct in_addr defaddr;
	static char namebuf[ 256 ];

	if ((sp = getservbyname("nntp", "tcp")) ==  NULL) {
		fprintf(stderr, "nntp/tcp: Unknown service.\n");
		return (-1);
	}
	/* If not a raw ip address, try nameserver */
	if (!isdigit(*machine) ||
	    (long)(defaddr.s_addr = inet_addr(machine)) == -1)
		hp = gethostbyname(machine);
	else {
		/* Raw ip address, fake  */
		(void) strcpy(namebuf, machine);
		def.h_name = namebuf;
#ifdef h_addr
		def.h_addr_list = alist;
#endif
		def.h_addr = (char *)&defaddr;
		def.h_length = sizeof(struct in_addr);
		def.h_addrtype = AF_INET;
		def.h_aliases = 0;
		hp = &def;
	}
	if (hp == NULL) {
		fprintf(stderr, "%s: Unknown host.\n", machine);
		return (-1);
	}

	bzero((char *) &sin, sizeof(sin));
	sin.sin_family = hp->h_addrtype;
	sin.sin_port = sp->s_port;
#else /* EXCELAN */
	bzero((char *) &sin, sizeof(sin));
	sin.sin_family = AF_INET;
#endif /* EXCELAN */

	/*
	 * The following is kinda gross.  The name server under 4.3
	 * returns a list of addresses, each of which should be tried
	 * in turn if the previous one fails.  However, 4.2 hostent
	 * structure doesn't have this list of addresses.
	 * Under 4.3, h_addr is a #define to h_addr_list[0].
	 * We use this to figure out whether to include the NS specific
	 * code...
	 */

#ifdef	h_addr

	/* get a socket and initiate connection -- use multiple addresses */

	for (cp = hp->h_addr_list; cp && *cp; cp++) {
		s = socket(hp->h_addrtype, SOCK_STREAM, 0);
		if (s < 0) {
			perror("socket");
			return (-1);
		}
	        bcopy(*cp, (char *)&sin.sin_addr, hp->h_length);
		
		if (x < 0)
			fprintf(stderr, "trying %s\n", inet_ntoa(sin.sin_addr));
		x = connect(s, (struct sockaddr *)&sin, sizeof (sin));
		if (x == 0)
			break;
                fprintf(stderr, "connection to %s: ", inet_ntoa(sin.sin_addr));
		perror("");
		(void) close(s);
	}
	if (x < 0) {
		fprintf(stderr, "giving up...\n");
		return (-1);
	}
#else	/* no name server */
#ifdef EXCELAN
	if ((s = socket(SOCK_STREAM,(struct sockproto *)NULL,&sin,SO_KEEPALIVE)) < 0)
	{
		/* Get the socket */
		perror("socket");
		return (-1);
	}
	bzero((char *) &sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(IPPORT_NNTP);
	/* set up addr for the connect */

	if ((sin.sin_addr.s_addr = rhost(&machine)) == -1) {
		fprintf(stderr, "%s: Unknown host.\n", machine);
		return (-1);
	}
	/* And then connect */

	if (connect(s, (struct sockaddr *)&sin) < 0) {
		perror("connect");
		(void) close(s);
		return (-1);
	}
#else /* not EXCELAN */
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return (-1);
	}

	/* And then connect */

	bcopy(hp->h_addr, (char *) &sin.sin_addr, hp->h_length);
	if (connect(s, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
		perror("connect");
		(void) close(s);
		return (-1);
	}

#endif /* !EXCELAN */
#endif /* !h_addr */
#endif /* !TLI */
	return (s);
}

#ifdef DECNET
/*
 * get_dnet_socket -- get us a socket connected to the news server.
 *
 *	Parameters:	"machine" is the machine the server is running on.
 *
 *	Returns:	Socket connected to the news server if
 *			all is ok, else -1 on error.
 *
 *	Side effects:	Connects to server.
 *
 *	Errors:		Printed via nerror.
 */

get_dnet_socket(machine)
char	*machine;
{
	int	s, area, node;
	struct	sockaddr_dn sdn;
	struct	nodeent *getnodebyname(), *np;

	bzero((char *) &sdn, sizeof(sdn));

	switch (s = sscanf( machine, "%d%*[.]%d", &area, &node )) {
		case 1: 
			node = area;
			area = 0;
		case 2: 
			node += area*1024;
			sdn.sdn_add.a_len = 2;
			sdn.sdn_family = AF_DECnet;
			sdn.sdn_add.a_addr[0] = node % 256;
			sdn.sdn_add.a_addr[1] = node / 256;
			break;
		default:
			if ((np = getnodebyname(machine)) == NULL) {
				fprintf(stderr, 
					"%s: Unknown host.\n", machine);
				return (-1);
			} else {
				bcopy(np->n_addr, 
					(char *) sdn.sdn_add.a_addr, 
					np->n_length);
				sdn.sdn_add.a_len = np->n_length;
				sdn.sdn_family = np->n_addrtype;
			}
			break;
	}
	sdn.sdn_objnum = 0;
	sdn.sdn_flags = 0;
	sdn.sdn_objnamel = strlen("NNTP");
	bcopy("NNTP", &sdn.sdn_objname[0], sdn.sdn_objnamel);

	if ((s = socket(AF_DECnet, SOCK_STREAM, 0)) < 0) {
		nerror("socket");
		return (-1);
	}

	/* And then connect */

	if (connect(s, (struct sockaddr *) &sdn, sizeof(sdn)) < 0) {
		nerror("connect");
		close(s);
		return (-1);
	}

	return (s);
}
#endif



/*
 * handle_server_response
 *
 *	Print some informative messages based on the server's initial
 *	response code.  This is here so inews, rn, etc. can share
 *	the code.
 *
 *	Parameters:	"response" is the response code which the
 *			server sent us, presumably from "server_init",
 *			above.
 *			"nntpserver" is the news server we got the
 *			response code from.
 *
 *	Returns:	-1 if the error is fatal (and we should exit).
 *			0 otherwise.
 *
 *	Side effects:	None.
 */

handle_server_response(response, nntpserver)
int	response;
char	*nntpserver;
{
    switch (response) {
	case OK_NOPOST:		/* fall through */
    		printf(
	"NOTE: This machine does not have permission to post articles.\n");
		printf(
	"      Please don't waste your time trying.\n\n");

	case OK_CANPOST:
		return (0);
		break;

	case ERR_ACCESS:
		printf(
   "This machine does not have permission to use the %s news server.\n",
		nntpserver);
		return (-1);
		break;

	default:
		printf("Unexpected response code from %s news server: %d\n",
			nntpserver, response);
		return (-1);
		break;
    }
	/*NOTREACHED*/
}


/*
 * put_server -- send a line of text to the server, terminating it
 * with CR and LF, as per ARPA standard.
 *
 *	Parameters:	"string" is the string to be sent to the
 *			server.
 *
 *	Returns:	Nothing.
 *
 *	Side effects:	Talks to the server.
 *
 *	Note:		This routine flushes the buffer each time
 *			it is called.  For large transmissions
 *			(i.e., posting news) don't use it.  Instead,
 *			do the fprintf's yourself, and then a final
 *			fflush.
 */

void
put_server(string)
char *string;
{
#ifdef DEBUG
	fprintf(stderr, ">>> %s\n", string);
#endif
#ifdef NOFDOPEN
	sockprintf(sockt, "%s\r\n", string);
#else
	fprintf(ser_wr_fp, "%s\r\n", string);
	(void) fflush(ser_wr_fp);
#endif
}


/*
 * get_server -- get a line of text from the server.  Strips
 * CR's and LF's.
 *
 *	Parameters:	"string" has the buffer space for the
 *			line received.
 *			"size" is the size of the buffer.
 *
 *	Returns:	-1 on error, 0 otherwise.
 *
 *	Side effects:	Talks to server, changes contents of "string".
 */

get_server(string, size)
char	*string;
int	size;
{
	register char *cp;
	char	*index();

#ifdef NOFDOPEN
    if (sockgets(string, size, sockt) == NULL)
#else
	if (fgets(string, size, ser_rd_fp) == NULL)
#endif
		return (-1);

	if ((cp = index(string, '\r')) != NULL)
		*cp = '\0';
	else if ((cp = index(string, '\n')) != NULL)
		*cp = '\0';
#ifdef DEBUG
	fprintf(stderr, "<<< %s\n", string);
#endif

	return (0);
}


/*
 * close_server -- close the connection to the server, after sending
 *		the "quit" command.
 *
 *	Parameters:	None.
 *
 *	Returns:	Nothing.
 *
 *	Side effects:	Closes the connection with the server.
 *			You can't use "put_server" or "get_server"
 *			after this routine is called.
 */

void
close_server()
{
	char	ser_line[256];

#ifdef NOFDOPEN
    if (sockt == -1)
#else
	if (ser_wr_fp == NULL || ser_rd_fp == NULL)
#endif
		return;

	put_server("QUIT");
	(void) get_server(ser_line, sizeof(ser_line));

#ifndef NOFDOPEN
	(void) fclose(ser_wr_fp);
	(void) fclose(ser_rd_fp);
#endif
}

