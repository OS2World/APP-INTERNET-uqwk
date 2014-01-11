#include <stdio.h>
#include <string.h>
#include "uqwk.h"

/*
 *  Miscellaneous uqwk routines
 */

char f[PATH_LEN];	/* Static area for ParseFrom() */

struct conf_ent *NewConference (name, num)
char *name;
int num;
{
	struct conf_ent *tmp_cp;
	char *c;

	/* Get space for new conference */
	if (NULL == (tmp_cp = (struct conf_ent *) malloc
					(sizeof (struct conf_ent))))
	{
		fprintf (stderr, "%s: out of memory\n", progname);
		exit (0);
	}

	/* Get space for name */
	if (NULL == (c = (char *) malloc (1+strlen(name))))
	{
		fprintf (stderr, "%s: out of memory\n", progname);
		exit (0);
	}

	/* Fill in conference name */
	tmp_cp->name = c;
	strcpy (tmp_cp->name, name);

	/* Fill in conference number */
	tmp_cp->number = num;

	/* Article count */
	tmp_cp->count = 0;

	/* Add to end of conference list */
	if (last_conf == NULL)
	{
		/* This is first conference */
		conf_list = tmp_cp;
	}
	else
	{
		last_conf->next = tmp_cp;
	}
	tmp_cp->next = NULL;
	last_conf = tmp_cp;

	if (slnp_mode)
	{
		/* Open SLNP message file */
		sprintf (msg_fn, "%s/%07d.MSG", home_dir, num);
		if (NULL == (msg_fd = fopen (msg_fn, "wb")))
		{
			fprintf (stderr, "%s: can't open %s\n",
					progname, msg_fn);
			exit (0);
		}
	}
	else if (!zip_mode && !sum_mode)
	{
		/* Else open new QWK index file */
		if (!bw_kludge && !strcmp (name, MAIL_CONF_NAME))
		{
			strcpy (ndx_fn, home_dir);
			strcat (ndx_fn, "/");
			strcat (ndx_fn, "personal.ndx");
		}
		else
		{
			sprintf (ndx_fn, "%s/%03d.ndx", home_dir, num);
		}

		if (NULL == (ndx_fd = fopen (ndx_fn, "wb")))
		{
			fprintf (stderr, "%s: can't open %s\n",
					progname, ndx_fn);
			exit (0);
		}
	}

	/* Maintain conf_cnt: should always be one greater than highest
	   newsgroup conference number encountered */
	if (strcmp (name, MAIL_CONF_NAME))
	{
		if (num >= conf_cnt) conf_cnt = num + 1;
	}

	/* Remember no summaries for this newsgroup yet */
	if (sum_mode) sum_flag = 0;

	return (tmp_cp);
}

PadString (s, c, n)
char *s, *c;
int n;
/*
 *  Take a null-terminated string s and copy it, space-padded or
 *  truncated if necessary, into field c of n characters
 */
{
	int len;
	len = strlen (s);
	if (len >= n)
	{
		strncpy (c, s, n);
	}
	else
	{
		strcpy (c, s);
		Spaces (&c[len], n-len);
	}
}

Spaces (c, n)
char *c;
int n;
/*
 *  Fill field of n characters with spaces
 */
{
	int i;
	for (i=0; i<n; i++) c[i]=' ';
}

PadNum (i, c, n)
int i, n;
char *c;
/*
 *  Format an integer i and place it, space filled, in
 *  field c of n characters
 */
{
	sprintf (buf, "%d", i);
	PadString (buf, c, n);
}

IntNum (i, c)
int i;
char c[2];
/*
 *  Put binary integer i into two bytes
 */
{
	c[0] = i % 256;
	c[1] = (i / 256) % 256;
}

char *Fgets (c, n, fd)
char *c;
int n;
FILE *fd;
/*
 *  Same as fgets, but changes trailing linefeed to a null
 */
{
	int i;

	if (NULL == fgets (c, n, fd)) return (NULL);
	i = strlen (c);
	if ( (i > 0) && (c[i-1]=='\n') ) c[i-1] = 0;
	if ( (i > 1) && (c[i-2]=='\r') ) c[i-2] = 0;

	return (c);
}

inttoms (i, c)
int i;
char c[4];
/*
 *  Convert an integer into the Microsoft Basic floating format.
 *  This is the dumbest thing in the whole QWK standard.  Why in
 *  the world store block offsets as floating point numbers?
 *  Stupid!
 */
{
	int m, e;

	if (i == 0)
	{
		c[0] = c[1] = c[2] = 0;
		c[3] = 0x80;
		return (0);
	}

	e = 152;
	m = 0x7fffff & i;

	while (!(0x800000 & m))
	{
		m <<= 1;
		e--;
	}
	c[0] = 0xff & m;
	c[1] = 0xff & (m >> 8);
	c[2] = 0x7f & (m >> 16);
	c[3] = 0xff & e;
	return (0);
}

ParseDate (c, hp)
char *c;
struct qwk_hdr *hp;
{
	char s[PATH_LEN];
	int day, mon, year, hour, minute;
	char month[4];

	/* Skip white space */
	while ( (*c == ' ') || (*c == 9) ) c++;

	/* Dates come in two flavors:  with the weekday, and without.
	   we simply look for the comma which follows the weekday */
	if (c[3] == ',')
	{
	        sscanf (&c[4], "%d %s %d %d:%d", &day, month, &year,
	                                        &hour, &minute);
	}
	else
	{
	        sscanf (c, "%d %s %d %d:%d", &day, month, &year,
	                                        &hour, &minute);
	}

	/* Convert alphabetic month name to integer */
	mon = 0;
	if (!strncmp (month, "Jan", 3)) mon = 1;
	if (!strncmp (month, "Feb", 3)) mon = 2;
	if (!strncmp (month, "Mar", 3)) mon = 3;
	if (!strncmp (month, "Apr", 3)) mon = 4;
	if (!strncmp (month, "May", 3)) mon = 5;
	if (!strncmp (month, "Jun", 3)) mon = 6;
	if (!strncmp (month, "Jul", 3)) mon = 7;
	if (!strncmp (month, "Aug", 3)) mon = 8;
	if (!strncmp (month, "Sep", 3)) mon = 9;
	if (!strncmp (month, "Oct", 3)) mon = 10;
	if (!strncmp (month, "Nov", 3)) mon = 11;
	if (!strncmp (month, "Dec", 3)) mon = 12;

	/* Convert date */
	sprintf (s, "%02d-%02d-%02d", mon, day, year%100);
	PadString (s, hp->date, 8);

	/* Time */
	sprintf (s, "%02d:%02d", hour, minute);
	PadString (s, hp->time, 5);
}

char *ParseFrom (c)
char *c;
/*
 *  Extract the email address from a From: line
 */
{
	int type, n, i, where;

	/*
	 *  Addresses come in three flavors:
	 *
	 *      1: seb3@gte.com
	 *      2: seb3@gte.com (steve belczyk)
	 *      3: steve belczyk <seb3@gte.com>
	 */

	/* Assume type 1 */
	type = 1;

	/* Look through address */
	n = strlen (c);
	for (i=0; i<n; i++)
	{
	        /* Change close-angle-bracket to null so we can
	           sscanf the address later */
	        if (c[i] == '>') c[i] = 0;

	        /* If we find an open-angle-bracket, assume type 3 */
	        if (c[i] == '<')
	        {
	                type = 3;
	                where = i+1;
	        }
	}

	/* Now extract the address */
	if (type == 1)
	{
	        /* This works for type 2 addresses too */
	        sscanf (c, "%s", f);
	}
	else    /* type == 3 */
	{
	        /* Check we don't fly off the end of c.  This should
	           never happen, but I've been wrong before. */
	        if (where > n)
	        {
	                strcpy (f, "unknown");
	        }
	        else
	        {
	                /* Get address */
	                sscanf (&c[where], "%s", f);
	        }
	}

	/* Done! */
	return (&f[0]);
}

