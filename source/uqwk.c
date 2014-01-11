#include <stdio.h>
#include <string.h>

#define ALLOCATE
#include "uqwk.h"

/*
 *  Try to make QWK packets from a mail and/or news spool
 */

main (argc, argv)
int argc;
char *argv[];
{
	progname = argv[0];

	/* Set up defaults */
	DefaultOptions();

	/* Look for environment variable overrides */
	EnvOptions();

	/* Look for command line overrides */
	CommandOptions (argc, argv);

	/* Initialize files, etc. */
	InitStuff();

	/* Do reply packet? */
	if (strcmp (rep_file, DEF_REP_FILE)) DoReply();

	/* Do news? */
	if (do_news) DoNews();

	/* Mail? */
	if (do_mail) DoMail();

	/* All done */
	CloseStuff();

	return (0);
}
