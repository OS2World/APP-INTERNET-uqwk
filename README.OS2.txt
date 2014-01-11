
                                 UQWK/2

                    Copyright 1993-1994, steve belczyk

                        OS/2 port by Claudio Fahey
                        claudio@uclink.berkeley.edu


INSTALLING:

    To install UQWK, copy UQWK.EXE, INEWS.EXE, and MAIL.CMD to a
    directory in your path.  You also need the emx run-time DLLs.  You 
    can get these from ftp-os2.nmsu.edu:/os2/2_x/unix/emx08h/emxrt.zip.
    Then make sure your environment variables are set correctly (see 
    below).  If you don't already have a .newsrc file in your home 
    directory, then create one.  

    To test it, run "uqwk +L -m +n +r".  This will create a SOUP packet
    in your home directory with news only.  It won't update your .newsrc
    file.

    Here's a typical batch file designed to import news into Yarn:
        cd \home
        uqwk +L -m +n
        zip -0m soup.zip *.msg areas
        import soup.zip
    And to export messages from Yarn:
        cd \temp
        unzip -o \home\soup.zip
        del \home\soup.zip
        uqwk +L -m -Rreplies
 
ENVIRONMENT VARIABLES:

    In addition to the environment variables mentioned in the man page,
    the following variables are used:

    HOME    Specifies the users home directory.  This is where the
            .newsrc file is located and where news packets are stored.
            This defaults to the current directory.

    USER    The user's login name.  This could be anything you want in OS/2.
            This defaults to "unknown"

    NNTPSERVER  Specifies the NNTP news server.  This is required unless news
                is being read from a news spool file.

    MAILER  Command to send mail.  This defaults to "sendmail".  If
            using LamPOP, this could also be set to "lampop -s myhost.domain
            myname mypasswd"

    POSTER  Command to post news.  This defaults to "inews".

    TEMP    Directory where temporary files are stored.  This defaults
            to the current directory.

LIMITATIONS:

    Receiving mail is not supported since OS/2 does not spool mail the 
    way Unix does.  

    Sending mail from ZipMail packets does not work because
    OS/2's sendmail command does not support a command of the form:
        sendmail 'joe@host.domain'
    This could probably easily be fixed by adding a few lines or using
    MAIL.CMD to add the To: address.

    The included UQWK.EXE was compiled with NNTP support.  This support 
    requires IBM TCP/IP 2.0.  

CHANGES TO THE SOURCE:

    Changed fopen modes to binary except for the following files:
        nrc_file    (.newsrc file)
        trn_file    (translate file)
        ng_file     (newsgroup file)
        sum_file    (summary file)
        sel_file    (selection file)

    Put temporary files in TEMPDIR instead of "/tmp".  TEMPDIR is
    defined as getenv(TEMP) or the current directory.  

    Sendmail and inews commands are taken from MAILER and POSTER
    environment variables, respectively.
 
    Modified reply.c to support the way the sendmail and inews paths
    and command line parameters are specified.  For example, the
    following line: 
        sprintf (buf, "%s", XPRT_MAILER);
    was changed to:
        sprintf (buf, "%s -t", SENDMAIL_PATH);

    Modified nntplib.c so that it accesses the socket directly instead
    of through a FILE stream.  This is because, in OS/2, fdopen can't be
    used to open a socket, only a file.  The file sockstream.c includes
    some functions such as sockgets() which should replace fgets() calls
    when referring to a socket.

BUGS, ETC:

    Feel free to send comments, bug reports, or suggestions related to 
    this OS/2 port to me.

    Claudio Fahey
    claudio@uclink.berkeley.edu

