#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include "uqwk.h"

#ifdef SERVER
#include "nntp.h"
extern void close_server();
#endif

/*
 *  Wrap things up
 */

CloseStuff()
{
	if (msg_fd != NULL) fclose (msg_fd);

	/* Write QWK CONTROL.DAT file (or SLNP AREAS file) */
	if (!zip_mode && !sum_mode && (do_mail || do_news)) WriteControl();

	/* Close summary file */
	if (sum_mode) fclose (sum_fd);

	/* Update .newsrc */
	if (do_news && (!read_only) ) WriteNewsrc();

	if ( (blk_cnt >= max_blks) && (max_blks > 0) )
	{
		fprintf (stderr,
			"%s: block count exceeded; some articles not packed\n",
			progname);
	}

	/* Remove reply packet */
	if ( (!read_only) && (strcmp (rep_file, DEF_REP_FILE)))
	{
		unlink (rep_file);
	}

#ifdef SERVER
	if (strcmp (rep_file, DEF_REP_FILE) || do_news) close_server();
#endif
}

WriteControl()
/*
 *  Create the CONTROL.DAT file (or AREAS if SLNP)
 */
{
	struct conf_ent *cp;
	struct tm *t;
	char ctl_fname[PATH_LEN];
	time_t clock;
	int n;

	strcpy (ctl_fname, home_dir);
	strcat (ctl_fname, "/");

	if (slnp_mode)
	{
		strcat (ctl_fname, "AREAS");
	}
	else
	{
		strcat (ctl_fname, "control.dat");
	}

	if (NULL == (ctl_fd = fopen (ctl_fname, "wb")))
	{
		fprintf (stderr, "%s: can't open %s\n", progname, ctl_fname);
		exit (0);
	}

	/* SLNP AREAS file is different */
	if (slnp_mode)
	{
		WriteAreas();
		return (0);
	}

	fprintf (ctl_fd, "%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n",
		bbs_name, bbs_city, bbs_phone, bbs_sysop, bbs_id);

	/* Date */
	clock = time (NULL);
	t = gmtime (&clock);
	fprintf (ctl_fd, "%02d-%02d-%04d,%02d:%02d:%02d\r\n",
		t->tm_mon+1, t->tm_mday, t->tm_year+1900,
		t->tm_hour, t->tm_min, t->tm_sec);

	fprintf (ctl_fd, "%s\r\n \r\n0\r\n", user_name);

	/* Count conferences with articles */
	n = 0;
	cp = conf_list;
	while (cp != NULL)
	{
		if (every_mode || (cp->count > 0)) n++;
		cp = cp->next;
	}
	fprintf (ctl_fd, "%d\r\n%d\r\n", msg_cnt, n-1);

	/* List of conferences */
	cp = conf_list;
	while (cp != NULL)
	{
		if (every_mode || (cp->count > 0))
		{
			strcpy (buf, cp->name);

			/* Translate the name if desired */
			if (trn_list != NULL) TransName (buf);

			/* Truncate the group name if desired */
			if ( (grp_len > 0) && (grp_len < BUF_LEN) )
			{
				buf[grp_len] = 0;
			}

			fprintf (ctl_fd, "%d\r\n%s\r\n", cp->number, buf);
		}
		cp = cp->next;
	}

	fprintf (ctl_fd, "WELCOME.DAT\r\nNEWS.DAT\r\nLOGOFF.DAT\r\n");
	fprintf (ctl_fd, "\032");
	fclose (ctl_fd);
	return (0);
}

WriteAreas()
/*
 *  Write the SLNP AREAS file
 */
{
	struct conf_ent *cp;

	/* Loop through conference list */
	cp = conf_list;
	while (cp != NULL)
	{
		if (strcmp (cp->name, MAIL_CONF_NAME))
		{
			if (cp->count > 0)
			{
				fprintf (ctl_fd, "%07d\011%s\011un\n",
					cp->number, cp->name);
			}
		}
		else
		{
			fprintf (ctl_fd, "%07d\011%s\011bn\n",
				cp->number, cp->name);
		}

		cp = cp->next;
	}
	fclose (ctl_fd);
	return (0);
}

WriteNewsrc()
/*
 *  Rewrite the updated .newsrc file
 */
{
	struct nrc_ent *np;

	if (read_only) return (0);

	if (NULL == (nrc_fd = fopen (nrc_file, "w")))
	{
		fprintf (stderr, "%s: can't write %s\n",
			progname, nrc_file);
		return (0);
	}

	for (np=nrc_list; np!=NULL; np=np->next)
	{
		/* Write this one */
		if (waf_mode)
		{
			if (np->subscribed)
			{
				fprintf (nrc_fd, "%s %d\r\n",
					np->name, np->sub->hi);
			}
		}
		else  /* Not waffle mode */
		{
			if (np->subscribed)
			{
				fprintf (nrc_fd, "%s: ", np->name);
			}
			else
			{
				fprintf (nrc_fd, "%s! ", np->name);
			}
			WriteSub (nrc_fd, np->sub);
		}
	}
	fclose (nrc_fd);
	return (1);
}

TransName (n)
char *n;
/*
 *  Translate newsgroup name
 */
{
	struct trn_ent *tp;

	tp = trn_list;

	while (tp != NULL)
	{
		if (!strcmp (n, tp->old))
		{
			/* Found a match */
			strcpy (n, tp->new);
			return (0);
		}
		tp = tp->next;
	}
	return (0);
}

