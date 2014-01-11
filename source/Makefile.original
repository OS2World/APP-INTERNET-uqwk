#
#  Makefile for uqwk
#
#  steve belczyk  02/93
#
CFLAGS=
CC=cc
#
#  Use this for SGI:
#CFLAGS=-cckr
#
#  Use this for SCO 3.2.2:
#CFLAGS=-Od

uqwk:	uqwk.o options.o init.o mail.o misc.o close.o news.o reply.o offline.o
	$(CC) -o uqwk uqwk.o options.o init.o mail.o misc.o close.o news.o reply.o offline.o

uqwk.o:	uqwk.c uqwk.h

options.o:	options.c uqwk.h

init.o:	init.c uqwk.h

mail.o:	mail.c uqwk.h

misc.o:	misc.c uqwk.h

close.o:	close.c uqwk.h

news.o:	news.c uqwk.h

reply.o:	reply.c uqwk.h

offline.o:	offline.c uqwk.h

clean:
	rm -f *.o uqwk

tar:
	tar cvf uqwk.tar *.[ch] Makefile* README uqwk.man uqwk.cat HISTORY door.id FAQ
	compress -v uqwk.tar

uu:
	uuencode uqwk.tar.Z uqwk.tar.Z >uqwk.uu
