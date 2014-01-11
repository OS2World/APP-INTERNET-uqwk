/* !CRF! - Mail.cmd */
/*
   This is a simple mail program for use with UQWK with parameters that
   are compatible with Unix's Mail.

   Only supports the following two command line forms:
        mail -s 'The Subject' joe@host.domain
        mail -s 'The Subject' 'joe@host.domain (Joe Smith)'

   The MAILER environment variable must be set.

   Written by Claudio Fahey (claudio@uclink.berkeley.edu)
*/

/* Load rexxutil SysTempFileName and SysFileDelete */
call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

parse arg "-s '" subject "' " to

if subject = '' | to = '' then do
    call lineout 'stderr:', 'MAIL.CMD: ERROR: Required subject or to-address not specified'
    exit 1
    end

/* See if to-address is enclosed in single quotes */
parse var to "'"temp"'"
if temp \= '' then
    to = temp

tempfile = systempfilename(value('TEMP',,'OS2ENVIRONMENT') || '\MailCmd.???')

call lineout tempfile, 'To:' to
call lineout tempfile, 'Subject:' subject

/* If any other headers need to be sent with the message, place them here */

call lineout tempfile, ''

/* copy message from stdin to temp file */
do while lines()
    call lineout tempfile, linein()
    end

/* Close temp file */
call lineout tempfile

/* Run MAILER (usually sendmail) */
'@'getenv('MAILER',,'OS2ENVIRONMENT') '-t <' tempfile
 
call sysfiledelete(tempfile)

