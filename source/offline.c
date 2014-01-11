#include <stdio.h>
#include <string.h>
#include "uqwk.h"
/*
 *  Process offline commands
 */

QWKOffLine (bytes, fd)
int bytes;
FILE *fd;
/*
 *  Process offline commands.  Message is open on fd.  We
 *  must be careful to leave the file pointer ready for the
 *  next message.
 */
{
	FILE *pfd;
	char c, cmd[PATH_LEN];

	/* Open mail pipe to send results back to user */
	sprintf (buf, "%s -s 'Results of your request' %s",
			MAILER_PATH, user_name);
	if (NULL == (pfd = popen (buf, "w")))
	{
		fprintf (stderr, "%s: can't popen() mail\n", progname);
		while (bytes--) fread (&c, 1, 1, fd);
		return (0);
	}

	fprintf (pfd, "Here are the results of your mail to UQWK:\n");

	/* Get lines, process them */
	while (GetLine (&bytes, fd))
	{
		/* Echo command */
		fprintf (pfd, "\nCommand: %s\n", buf);

		/* Extract command */
		if (1 != sscanf (buf, "%s", cmd))
		{
			fprintf (pfd, "Malformed command.\n");
		}
		else
		{
			/* Look up command */
			if ( (!strcmp (cmd, "help")) ||
			     (!strcmp (cmd, "HELP")) )
			{
				Help(pfd);
			}
			else if ( (!strcmp (cmd, "subscribe")) ||
				  (!strcmp (cmd, "SUBSCRIBE")) )
			{
				Subscribe(pfd);
			}
			else if ( (!strcmp (cmd, "unsubscribe")) ||
				  (!strcmp (cmd, "UNSUBSCRIBE")) )
			{
				Unsubscribe(pfd);
			}
			else if ( (!strcmp (cmd, "groups")) ||
			          (!strcmp (cmd, "GROUPS")) )
			{
				Groups(pfd);
			}
			else if ( (!strcmp (cmd, "allgroups")) ||
			          (!strcmp (cmd, "ALLGROUPS")) )
			{
				Allgroups(pfd);
			}
			else if ( (!strcmp (cmd, "shell")) ||
				  (!strcmp (cmd, "SHELL")) )
			{
				Shell(pfd);
			}
			else if ( (!strcmp (cmd, "catchup")) ||
				  (!strcmp (cmd, "CATCHUP")) )
			{
				Catchup(pfd);
			}
			else
			{
				fprintf (pfd, "No such command.  ");
				fprintf (pfd, "Send HELP for help.\n");
			}
		}
	}

	fprintf (pfd, "\nEnd of commands.\n");
	pclose (pfd);
	return (1);
}

int GetLine (bytes, fd)
int *bytes;
FILE *fd;
/*
 *  Get a line from fd, put it in buf, check for end of message
 */
{
	int i;
	unsigned char eol;

	i = 0;

	eol = QWK_EOL;
	if (slnp_mode || zip_mode) eol = '\n';

	/* Read bytes until EOL or end of message */
	while (*bytes)
	{
		fread (&buf[i], 1, 1, fd);
		(*bytes)--;

		/* Lose CR's from ZipNews */
		if (buf[i] == '\r') buf[i] = 0;

		if ( (buf[i] == eol) || (i == BUF_LEN-1) )
		{
			buf[i] = 0;
			return (1);
		}
		i++;
	}

	/* If we got here, we ran out of bytes */
	return (0);
}

Help (pfd)
FILE *pfd;
{
	fprintf (pfd, "\nAvailable commands:\n\n");
	fprintf (pfd, "HELP - This message.\n");
	fprintf (pfd, "SUBSCRIBE newsgroup - Subscribe to named newsgroup.\n");
	fprintf (pfd, "UNSUBSCRIBE newsgroup - Unsubscribe from newsgroup.\n");
	fprintf (pfd, "UNSUBSCRIBE ALL - Unsubscribe from all newsgroups.\n");
	fprintf (pfd, "GROUPS - List all subscribed newsgroups.\n");
	fprintf (pfd, "ALLGROUPS - List all available newsgroups.\n");
	fprintf (pfd, "CATCHUP newsgroup - Mark all articles as read.\n");
	fprintf (pfd, "SHELL command - Execute shell command\n\n");
}

Subscribe (pfd)
FILE *pfd;
{
	struct act_ent *ap;
	struct nrc_ent *np;
	char group[PATH_LEN];

	/* Extract group name */
	if (1 != sscanf (buf, "%*s %s", group))
	{
		fprintf (pfd, "Usage: SUBSCRIBE newsgroup\n");
		return (0);
	}

	/* We will need active file and .newsrc */
	if (!ReadActive() || !ReadNewsrc())
	{
		fprintf (pfd, "Sorry, couldn't read system files.\n");
		return (0);
	}

	/* Already subscribed? */
	np = nrc_list;
	while (np != NULL)
	{
		if (!strcmp (group, np->name))
		{
			if (np->subscribed)
			{
				fprintf (pfd, "Already subscribed to %s.\n",
					group);
				return (0);
			}
			else
			{
				np->subscribed = 1;
				fprintf (pfd, "Okay, re-subscribed to %s.\n",
					group);
				WriteNewsrc();
				return (0);
			}
		}
		np = np->next;
	}

	/* Find group in active file */
	if (NULL == (ap = FindActive (group)))
	{
		fprintf (pfd, "No such newsgroup: %s\n", group);
		return (0);
	}

	/* Okay already, add to .newsrc */
	np = (struct nrc_ent *) malloc (sizeof (struct nrc_ent));
	if (np == NULL) OutOfMemory();
	np->name = (char *) malloc (1+strlen(group));
	if (np->name == NULL) OutOfMemory();
	strcpy (np->name, group);
	np->subscribed = 1;

	/* Make subscription list - everything is read */
	if (NULL == (np->sub = (struct sub_ent *) malloc
			(sizeof (struct sub_ent)))) OutOfMemory();
	np->sub->lo = 1;
	np->sub->hi = ap->hi;
	np->sub->next = NULL;

	np->next = nrc_list;
	nrc_list = np;

	WriteNewsrc();
	fprintf (pfd, "Okay, you are now subscribed to %s.\n", group);

	return (1);
}

Unsubscribe (pfd)
FILE *pfd;
{
	struct nrc_ent *np;
	char group[PATH_LEN];

	/* Parse group name */
	if (1 != sscanf (buf, "%*s %s", group))
	{
		fprintf (pfd, "Usage: UNSUBSCRIBE newsgroup\n");
		return (0);
	}

	/* Check for ALL */
	if ( (!strcmp (group, "ALL")) || (!strcmp (group, "all")) )
	{
		nrc_list = NULL;
		WriteNewsrc();
		fprintf (pfd,
		  "Okay, you are now unsubscribed from all newsgroups.\n");
		return (0);
	}

	/* We need the .newsrc file */
	if (!ReadNewsrc())
	{
		fprintf (pfd, "Sorry, couldn't read .newsrc\n");
		return (0);
	}

	/* Look for group in newsrc */
	np = nrc_list;
	while (np != NULL)
	{
		if (!strcmp (group, np->name)) break;
		np = np->next;
	}

	if (np == NULL)
	{
		fprintf (pfd, "You are not currently subscribed to %s.\n",
		         group);
		return (0);
	}

	np->subscribed = 0;

	WriteNewsrc();
	fprintf (pfd, "Okay, you are unsubscribed from %s.\n", group);

	return (1);
}

Groups (pfd)
FILE *pfd;
{
	struct nrc_ent *np;

	if (!ReadNewsrc())
	{
		fprintf (pfd, "Sorry, couldn't read .newsrc\n");
		return (0);
	}

	fprintf (pfd, "Newsgroups to which you are subscribed:\n\n");

	np = nrc_list;
	while (np != NULL)
	{
		if (np->subscribed) fprintf (pfd, "    %s\n", np->name);
		np = np->next;
	}
	return (1);
}

Allgroups (pfd)
FILE *pfd;
{
	struct act_ent *ap;

	if (!ReadActive())
	{
		fprintf (pfd, "Sorry, no newsgroups are available.\n");
		return (0);
	}

	fprintf (pfd, "List of available newsgroups:\n\n");

	ap = act_list;
	while (ap != NULL)
	{
		fprintf (pfd, "    %s (%d articles)\n",
			ap->name, ap->hi - ap->lo);
		ap = ap->next;
	}
	return (1);
}

Catchup (pfd)
FILE *pfd;
{
	struct act_ent *ap;
	struct nrc_ent *np;
	struct sub_ent *sp, *tsp;
	char group[PATH_LEN];

	/* Extract group name */
	if (1 != sscanf (buf, "%*s %s", group))
	{
		fprintf (pfd, "Usage: CATCHUP newsgroup\n");
		return (0);
	}

	/* We will need active file and .newsrc */
	if (!ReadActive() || !ReadNewsrc())
	{
		fprintf (pfd, "Sorry, couldn't read system files.\n");
		return (0);
	}

	/* Not subscribed? */
	np = nrc_list;
	while (np != NULL)
	{
		if (!strcmp (group, np->name))
		{
			if (np->subscribed)
			{
				break;
			}
			else
			{
				fprintf (pfd,
					"You are not subscribed to %s.\n",
					group);
				return (0);
			}
		}
		np = np->next;
	}

	if (np == NULL)
	{
		fprintf (pfd, "You are not subscribed to %s.\n", group);
		return (0);
	}

	/* Find group in active file */
	if (NULL == (ap = FindActive (group)))
	{
		fprintf (pfd, "No such newsgroup: %s\n", group);
		return (0);
	}

	/* Free subscription list */
	sp = np->sub;
	while (sp != NULL)
	{
		tsp = sp->next;
		free (sp);
		sp = tsp;
	}

	/* Okay already, add to .newsrc */
	np = (struct nrc_ent *) malloc (sizeof (struct nrc_ent));
	if (np == NULL) OutOfMemory();
	np->name = (char *) malloc (1+strlen(group));
	if (np->name == NULL) OutOfMemory();
	strcpy (np->name, group);
	np->subscribed = 1;

	/* Make subscription list - everything is read */
	if (NULL == (np->sub = (struct sub_ent *) malloc
			(sizeof (struct sub_ent)))) OutOfMemory();
	np->sub->lo = 1;
	np->sub->hi = ap->hi;
	np->sub->next = NULL;

	np->next = nrc_list;
	nrc_list = np;

	WriteNewsrc();
	fprintf (pfd, "Okay, you are now caught up in %s.\n", group);

	return (1);
}

Shell (pfd)
FILE *pfd;
{
	int c;
	FILE *cfd;

	if (strlen(buf) < 7)
	{
		fprintf (pfd, "Usage: SHELL command\n");
		return (0);
	}

	if (NULL == (cfd = popen (&buf[6], "r")))
	{
		fprintf (pfd, "Can't open pipe for command\n");
		return (0);
	}

	while (EOF != (c = fgetc (cfd)))
	{
		fputc ( (0xff & c), pfd);
	}
	fputc ('\n', pfd);
	pclose (cfd);
	return (1);
}

OffLine (fd, bytes)
FILE *fd;
int bytes;
/*
 *  Offline command
 */
{
	int n, read;
	char *rc;
	long offset;

	/* Skip header */
	offset = ftell (fd);		/* Remember where we are */
	rc = Fgets (buf, BUF_LEN, fd);
	n = strlen (buf);
	read = ftell(fd) - offset;	/* Compute bytes read */
	while ( (rc != NULL) && (bytes > 0) && (n > 0) )
	{
		/* Get next line */
		bytes -= read;
		offset = ftell (fd);
		rc = Fgets (buf, BUF_LEN, fd);
		read = ftell(fd) - offset;
		if (rc != NULL) n = strlen (buf);
	}

	/* Hand off to the QWK Offline function */
	QWKOffLine (bytes, fd);
}

