UQWK

Copyright 1993-1994, steve belczyk

uqwk is a program which collects all a user's unread mail or news
and formats it into a packet for offline reading.  QWK, Simple
Offline Usenet Packet (SOUP), and ZipNews packet formats are supported.

Uqwk also accepts reply packets, so replies can be mailed or posted,
depending whether the message is marked private (email) or public (news).

Uqwk also supports a small offline command language, so the contents
of the user's .newsrc file can be viewed and manipulated offline.

INSTALLATION

1.  Create a directory for the uqwk source code.

	mkdir /usr/local/src/uqwk

2.  Move the uqwk tar file to that directory.

	mv uqwk.tar.Z /usr/local/src/uqwk

3.  Change to that directory, and unpack the tar file.

	cd /usr/local/src/uqwk; zcat uqwk.tar | tar xvf -

4.  Compile the software.

	make

    If you're going to be getting news from a server using NNTP,
    you must use the NNTP version of the Makefile:

	make -f Makefile.nntp

    If this doesn't work you may have to twiddle with the Makefile,
    or, heaven forbid, the actual source code.

5.  Move the binary and man pages to some public place.

	mv uqwk /usr/local/bin
	mv uqwk.man /usr/man/man1/uqwk.1
	mv uqwk.cat /usr/man/cat1/uqwk.1

6.  The QWK specification allows for the name, location, etc., of
    the BBS from which messages are being downloaded.  If you plan
    to use the QWK format, you should configure this information.
    The best way is probably to use environment variables.  If you
    are using a Bourne shell, you should add something like this to
    /etc/profile or .profile:

        UQ_BBS_NAME="My Super BBS"
	UQ_BBS_CITY="Somewhere, PA"
	UQ_BBS_PHONE="+1 215 555 1212"
	UQ_BBS_SYSOP="Joe Shmoe"
	UQ_BBS_ID="0,MYBBS"
	export UQ_BBS_NAME UQ_BBS_CITY UQ_BBS_PHONE
	export UQ_BBS_SYSOP UQ_BBS_ID

    If you use a C type shell, try something like this in your
    .cshrc or .login:

        setenv UQ_BBS_NAME "My Super BBS"
	setenv UQ_BBS_CITY "Somewhere, PA"
	setenv UQ_BBS_PHONE "+1 215 555 1212"
	setenv UQ_BBS_SYSOP "Joe Shmoe"
	setenv UQ_BBS_ID "0,MYBBS"

    In both cases, the last entry, the "BBS ID", is the most important.
    It always consists of an integer, a comma, and a string less than
    nine characters, with no intervening spaces.  The string will be
    used to identify reply packets.

7.  Now you can try it.  Log in as a normal user who has some mail,
    and issue:

	uqwk +r

    (The "+r" stops uqwk from clearing the user's mail spool file.)
    This should create three new files in the current directory named
    messages.dat, control.dat, and personal.ndx.  These are the files
    which the offline reader will need.  (Not all readers need the
    *.ndx files.  Also, some readers expect these files to have been
    archived using an archiver like zip, lharc, or arj.  You may need
    to obtain Unix versions of these archivers.)

8.  Now it would be a good idea to read the man page ("more uqwk.cat")
    and the Frequently Asked Questions list ("more FAQ").

Please inform me of any problems you run into:

    steve belczyk
    steve1@genesis.nred.ma.us
    seb3@gte.com
    BBS: +1 508 664 0149

