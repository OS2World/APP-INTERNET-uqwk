#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "uqwk.h"

#define MAXFILENAME 128

#ifdef SERVER
#include "nntp.h"
extern FILE *getactive_nntp(); 
extern void group_nntp();
extern FILE *nntpopen();
#endif

/*
 *  All sorts of stuff to do news processing
 */

DoNews ()
/*
 *  Collect unread news into QWK packet
 */
{
	char active_name[MAXFILENAME];
	struct act_ent *ap;
	struct nrc_ent *np;
	struct ng_ent *ngp;

	/* Open ZipNews files */
	if (zip_mode)
	{
		if (!OpenZipFiles()) return (0);
	}

	/* Read .newsrc file */
	if (!ReadNewsrc()) return (0);

	/* And active file */
	if (!ReadActive()) return (0);

	/* Handle selection mode */
	if (sel_mode)
	{
		DoSelect ();
#ifdef SERVER
		sprintf(active_name,"%s/rrnact.%d",TEMPDIR,getpid());
		unlink(active_name);
#endif
		/* ZipNews "join" file */
		if (zip_mode)
		{
			WriteJoin ();
			fclose (nws_fd);
			fclose (idx_fd);
		}
		return (0);
	}

	/* Use .newsrc or desire newsgroup file? */
	if (ng_list == NULL)
	{
		/*
		 *  No desired ng file, use newsrc
		 */
		np = nrc_list;
		while (np != NULL)
		{
			/* Check if too many blocks already */
			if ( (blk_cnt >= max_blks) && (max_blks > 0) )
			{
#ifdef SERVER
				sprintf(active_name,"%s/rrnact.%d",TEMPDIR,getpid());
				unlink(active_name);
#endif
				/* ZipNews "join" file */
				if (zip_mode)
				{
					WriteJoin ();
					fclose (nws_fd);
					fclose (idx_fd);
				}
				return (0);
			}

			if (np->subscribed)
			{
				/* Lookup group in active file */
				ap = FindActive (np->name);

				/* Do this group */
				DoGroup (np, ap);
			}
			np = np->next;
		}
#ifdef SERVER
		sprintf(active_name,"%s/rrnact.%d",TEMPDIR,getpid());
		unlink(active_name);
#endif
		/* ZipNews "join" file */
		if (zip_mode)
		{
			WriteJoin ();
			fclose (nws_fd);
			fclose (idx_fd);
		}
	}
	else
	{
		/*
		 *  Read desired ng file for groups
		 */
		for (ngp=ng_list; ngp!=NULL; ngp=ngp->next)
		{
			/* Check if too many blocks already */
			if ( (blk_cnt >= max_blks) && (max_blks > 0) )
			{
#ifdef SERVER
				sprintf(active_name,"%s/rrnact.%d",TEMPDIR,getpid());
				unlink(active_name);
#endif
				/* ZipNews "join" file */
				if (zip_mode)
				{
					WriteJoin ();
					fclose (nws_fd);
					fclose (idx_fd);
				}
				return (0);
			}

			/* Find .newsrc entry, book if none */
			for (np=nrc_list; np!=NULL; np=np->next)
			{
				if (!strcmp (ngp->name, np->name)) break;
			}

			if (np == NULL)
			{
				fprintf (stderr, "%s: %s not in .newsrc\n",
						progname, ngp->name);
			}
			else
			{
				/* Lookup group in active file */
				ap = FindActive (np->name);

				/* Do this group */
				DoGroup (np, ap);
			}
		}
#ifdef SERVER
		sprintf(active_name,"%s/rrnact.%d",TEMPDIR,getpid());
		unlink(active_name);
#endif
		/* ZipNews "join" file */
		if (zip_mode)
		{
			WriteJoin ();
			fclose (nws_fd);
			fclose (idx_fd);
		}
	}

	return (1);
}

int ReadNewsrc()
/*
 *  Read the .newsrc file
 */
{
	char group_name[PATH_LEN];
	struct nrc_ent *np, *lnp;
	int i, n, c;

	/* lnp points to last entry */
	lnp = NULL;

	/* Don't bother if we've alread read it */
	if (nrc_list != NULL) return (1);

	/* Open it */
	if (NULL == (nrc_fd = fopen (nrc_file, "r")))
	{
		fprintf (stderr, "%s: can't open %s\n", progname, nrc_file);
		return (0);
	}

	/* Read through */
	while (NULL != Fgets (buf, BUF_LEN, nrc_fd))
	{
		/* Allocate a new nrc entry */
		np = (struct nrc_ent *) malloc (sizeof (struct nrc_ent));
		if (np == NULL) OutOfMemory();

		if (waf_mode)
		{
			/* Waffle is easy */
			sscanf (buf, "%s %d", group_name, &n);
			np->subscribed = 1;

			/* Allocate sub entry */
			if (NULL == (np->sub = (struct sub_ent *) malloc
				(sizeof (struct sub_ent)))) OutOfMemory();

			np->sub->lo = 0;
			np->sub->hi = n;
			np->sub->next = NULL;
		}
		else  /* Regular Unix-style .newsrc */
		{
			/* Assume not subscribed */
			np->subscribed = 0;

			/* Parse group name */
			n = strlen (buf);
			for (i=0; i<n; i++)
			{
				/* Some .newsrc's don't have a space after the
			 	   newsgroup name, so kludge it like this */
				if (buf[i] == ':')
				{
					np->subscribed = 1;
					buf[i] = ' ';

					/* Parse subscription list */
					np->sub = SubList (&buf[i+1]);
				}
				if (buf[i] == '!')
				{
					np->subscribed = 0;
					buf[i] = ' ';

					/* Parse subscription list */
					np->sub = SubList (&buf[i+1]);
				}
			}
			sscanf (buf, "%s", group_name);
		}

		n = strlen (group_name);

		np->name = (char *) malloc (n+1);
		if (np->name == NULL) OutOfMemory();
		strcpy (np->name, group_name);
		np->next = NULL;

		/* Add to nrc list */
		if (lnp == NULL)
		{
			/* First one */
			nrc_list = np;
		}
		else
		{
			/* Add to end */
			lnp->next = np;
		}
		lnp = np;
	}

	/* Walk through the nrc list, assign conference numbers */
	np = nrc_list;
	c = 0;
	while (np != NULL)
	{
		if (np->subscribed)
		{
			np->conf = c;
			c++;
		}
		else
		{
			np->conf = (-1);
		}
		np = np->next;
	}

	fclose (nrc_fd);
	return (1);
}

int ReadActive()
/*
 *  Read active file
 */
{
	char group_name[PATH_LEN];
	struct act_ent *ap;

	/* Don't bother if it's already here */
	if (act_list != NULL) return (1);

#ifdef SERVER
	/* retrieve active file from news server */
	if (NULL == (act_fd = getactive_nntp()))
	{
		fprintf (stderr, "%s: can't open %s\n", progname, act_file);
		return (0);
	}
#else
	/* Open the active file */
	if (NULL == (act_fd = fopen (act_file, "rb")))
	{
		fprintf (stderr, "%s: can't open %s\n", progname, act_file);
		return (0);
	}
#endif

	/* Read through it */
	while (NULL != Fgets (buf, BUF_LEN, act_fd))
	{
		/* Get new act entry */
		ap = (struct act_ent *) malloc (sizeof (struct act_ent));
		if (ap == NULL) OutOfMemory();

		/* Parse name, message numbers */
		sscanf (buf, "%s %d %d", group_name, &ap->hi, &ap->lo);
		ap->name = (char *) malloc (1+strlen(group_name));
		if (ap->name == NULL) OutOfMemory();
		strcpy (ap->name, group_name);

		/* Add to list */
		ap->next = act_list;
		act_list = ap;
	}
	fclose (act_fd);
	return (1);
}

struct act_ent *FindActive (c)
char *c;
/*
 *  Look for group's active entry given group name
 */
{
	struct act_ent *ap;

	ap = act_list;
	while (NULL != ap)
	{
		if (!strcmp (c, ap->name)) return (ap);
		ap = ap->next;
	}
	return (NULL);
}

DoGroup (np, ap)
struct nrc_ent *np;
struct act_ent *ap;
/*
 *  Process given group
 */
{
	char news_path[PATH_LEN];
	int i, n;
	struct conf_ent *cp;

	printf ("%s: %s\n", progname, np->name);

	/* Make a new conference with this name */
	cp = NewConference (np->name, np->conf);

	/* Remember no ZipNews index entry yet */
	if (zip_mode) zip_flag = 0;

#ifdef SERVER
	/* select group name from news server */
	group_nntp(np->name);
#else
	/* Construct path name for articles in this group */
	strcpy (news_path, news_dir);
	strcat (news_path, "/");
	strcat (news_path, np->name);
	strcat (news_path, "/");
	n = strlen (news_path);
	for (i=0; i<n; i++) if (news_path[i] == '.') news_path[i] = '/';
#endif

	/* If the group doesn't exist (that is, doesn't appear in
	   the active file), do nothing else */
	if (ap == NULL)
	{
		if (!slnp_mode && !zip_mode && !sum_mode) NdxClose (ndx_fd);
		if (slnp_mode) MsgClose (msg_fd);
		return (0);
	}

	/* Fix up the subscription list */
	np->sub = FixSub (np->sub, ap->lo, ap->hi);

	/* Look through unread articles */
	for (i=ap->lo; i<=ap->hi; i++)
	{
		/* Check max block count */
		if ( (blk_cnt >= max_blks) && (max_blks > 0) )
		{
			if (!slnp_mode && !zip_mode && !sum_mode)
							NdxClose (ndx_fd);
			if (slnp_mode) MsgClose (msg_fd);
			return (0);
		}

		/* Process this article */
		if (!IsRead (i, np->sub))
		{
			/* Mark as read */
			np->sub = MarkRead (i, np->sub);

			/* Process the article */
			DoArticle (news_path, i, np, cp);
		}
	}
	if (!slnp_mode && !zip_mode && !sum_mode) NdxClose (ndx_fd);
	if (slnp_mode) MsgClose (msg_fd);

	return (1);
}

DoArticle (news_path, artnum, np, cp)
char *news_path;
int artnum;
struct nrc_ent *np;
struct conf_ent *cp;
{
	struct qwk_hdr hdr;
	struct stat stat_buf;
	long txt_offset, end_offset;
	int n, out_bytes;
	char ndx[5], *eof;
	FILE *art_fd;
	char art_file[PATH_LEN];

#ifdef SERVER
	/* retrieve article from news server */
/*	printf("retrieving article %d\n",artnum); */
	art_fd = nntpopen(artnum, GET_ARTICLE);
	if (art_fd == NULL) return (0);

	/* Construct article's file name */ 
    	sprintf(art_file,"%s/rrn%ld.%ld", TEMPDIR, (long) artnum, getpid());
#else
	/* Construct article's file name */ 
	sprintf (art_file, "%s%d", news_path, artnum);

	/* Forget it if we can't open the article */
	if (NULL == (art_fd = fopen (art_file, "rb"))) return (0);

#endif
	/* stat() the article to get file size */
	if (0 != stat (art_file, &stat_buf))
	{
		fclose (art_fd);
#ifdef SERVER
		unlink (art_file);
#endif
		return (0);
	}

	end_offset = stat_buf.st_size;

	/* Skip empty articles */
	if (end_offset == 0)
	{
		fclose (art_fd);
#ifdef SERVER
		unlink (art_file);
#endif
		return (0);
	}

	/* We now assume the article is for real, so we can
	   bump this conference's article count */
	cp->count++;

	/* Check the Xref line if desired */
	if (xrf_mode) DoXref (art_fd, end_offset);

	/* Do SLNP stuff */
	if (slnp_mode)
	{
		SLNPArticle (art_fd, end_offset);
		fclose (art_fd);
#ifdef SERVER
		unlink(art_file);
#endif
		return (1);
	}

	/* Do ZipNews stuff */
	if (zip_mode)
	{
		ZipArticle (art_fd, end_offset, np);
		fclose (art_fd);
#ifdef SERVER
		unlink(art_file);
#endif
		return (1);
	}

	/* Do summary stuff */
	if (sum_mode)
	{
		SumArticle (art_fd, artnum, end_offset, np);
		fclose (art_fd);
#ifdef SERVER
		unlink(art_file);
#endif
		return (1);
	}

	/* Write the index file entry */
	inttoms (blk_cnt, ndx);
	ndx[4] = np->conf;
	fwrite (ndx, 5, 1, ndx_fd);

	Spaces (&hdr, 128);

	/* Fill in some header fields */
	hdr.status = QWK_PUBLIC;
	PadNum (msg_cnt, hdr.number, 7);
	Spaces (hdr.password, 12);
	Spaces (hdr.refer, 8);
	hdr.flag = QWK_ACT_FLAG;
	IntNum (np->conf, hdr.conference);
	IntNum (msg_cnt+1, hdr.msg_num);
	hdr.tag = ' ';
	PadString ("ALL", hdr.to, 25);

	msg_cnt++;

	/* Process header lines */
	eof = Fgets (buf, BUF_LEN, art_fd);
	while ( (0 != strlen(buf)) && (eof != NULL) )
	{
		if (!strncmp (buf, "Date: ", 6))
		{
			ParseDate (&buf[6], &hdr);
		}
		else if (!strncmp (buf, "Subject: ", 9))
		{
			PadString (&buf[9], hdr.subject, 25);
		}
		else if (!strncmp (buf, "From: ", 6))
		{
			PadString (ParseFrom(&buf[6]), hdr.from, 25);
		}

		eof = Fgets (buf, BUF_LEN, art_fd);
	}

	txt_offset = ftell (art_fd);

	/* Compute block count */
	if (inc_hdrs)
	{
		PadNum (2+end_offset/128, hdr.blocks, 6);
		blk_cnt += 1+end_offset/128;
	}
	else
	{
		PadNum (2+(end_offset-txt_offset)/128, hdr.blocks, 6);
		blk_cnt += 1+(end_offset-txt_offset)/128;
	}

	/* Write the message header */
	fwrite (&hdr, 128, 1, msg_fd);
	blk_cnt++;

	/* Now write the article's text */
	if (inc_hdrs) fseek (art_fd, 0, 0);
	out_bytes = 0;

	while (NULL != Fgets (buf, BUF_LEN, art_fd))
	{
		n = strlen (buf);
		fwrite (buf, n, 1, msg_fd);
		out_bytes += n;

		if (n < BUF_LEN-1)
		{
			fputc (QWK_EOL, msg_fd);
			out_bytes++;
		}
	}

	/* Pad block as necessary */
	n = out_bytes % 128;
	for (;n<128;n++) fputc (' ', msg_fd);

	fclose (art_fd);
#ifdef SERVER
	unlink(art_file);
#endif

	return (1);
}

SLNPArticle (art_fd, bytes)
FILE *art_fd;
int bytes;
/*
 *  Convert an article to SLNP format
 */
{
        int c;

        /* Write "rnews" line */
        fprintf (msg_fd, "#! rnews %d\n", bytes);

	/* Maintain block count */
	blk_cnt += (bytes + 64) / 128;

        /* Copy bytes */
        while (bytes--)
        {
                c = fgetc (art_fd);
                fputc ((0xff & c), msg_fd);
        }
        return (1);
}

ZipArticle (art_fd, bytes, np)
FILE *art_fd;
int bytes;
struct nrc_ent *np;
/*
 *  Convert article to ZipNews format
 */
{
	int c;
	long offset;

	/* Write separator */
	for (c=0; c<20; c++) fputc (1, nws_fd);
	fprintf (nws_fd, "\r\n");

	/* Write index file entry for this group if there isn't
	   already one */
	if (!zip_flag)
	{
		offset = ftell (nws_fd);
		fprintf (idx_fd, "N %08d %s\r\n", offset, np->name);
		zip_flag = 1;
	}

	/* Maintain block count */
	blk_cnt += (bytes + 64) / 128;

        /* Copy bytes */
        while (bytes--)
        {
                c = fgetc (art_fd);

		/* ZipNews doesn't like ^Z's */
		if (c == 26) c = 32;

		/* Map LF to CRLF */
		if (c == 10) fputc (13, nws_fd);

                fputc ((0xff & c), nws_fd);
        }
        return (1);
}

OutOfMemory()
{
	fprintf (stderr, "%s: out of memory\n", progname);
	exit (0);
}

struct sub_ent *SubList (c)
char *c;
/*
 *  Parse a newsrc subscription line, make into subscription list
 */
{
	struct sub_ent *sp, *sub_list;
	int lo, hi;
	char *range;

	/* Initialize subscription list */
	sub_list = NULL;

	/* Loop through line entries */
	range = strtok (c, ",");
	while (range != NULL)
	{
		/* Get space for new list entry */
		if (NULL == (sp = (struct sub_ent *)
			malloc (sizeof (struct sub_ent)))) OutOfMemory();

		/* Determine if it's a range or single entry */
		if (2 == sscanf (range, "%d-%d", &lo, &hi))
		{
			sp->lo = lo;
			sp->hi = hi;

			/* Reverse them in case they're backwards */
			if (hi < lo)
			{
				sp->lo = hi;
				sp->hi = lo;
			}
		}
		else	/* Not a range */
		{
			sp->lo = atoi (range);
			sp->hi = sp->lo;
		}

		/* Check if range overlaps last one */
		if ( (sub_list != NULL) && (sp->lo <= (sub_list->hi + 1)))
		{
			/* Combine ranges */
			if (sp->lo < sub_list->lo) sub_list->lo = sp->lo;
			if (sp->hi > sub_list->hi) sub_list->hi = sp->hi;

			/* Free old one */
			free (sp);
		}
		else
		{
			/* No overlap, maintain pointers */
			sp->next = sub_list;
			sub_list = sp;
		}

		range = strtok (NULL, ",");
	}

	return (sub_list);
}

int IsRead (num, sp)
int num;
struct sub_ent *sp;
/*
 *  Determine if an article has been read
 */
{
	/* Remember the list is from hi number to lo number */

	/* Look through the list */
	while (sp != NULL)
	{
		if (num > sp->hi) return (0);
		if ( (num >= sp->lo) && (num <= sp->hi) ) return (1);

		sp = sp->next;
	}

	return (0);
}

struct sub_ent *MarkRead (num, sp_head)
int num;
struct sub_ent *sp_head;
/*
 *  Mark article as read
 *
 *  Remember, the list is stored from highest numbers to lowest numbers.
 */
{
	struct sub_ent *sp, *tsp, *lsp;

	sp = sp_head;

	/* If num is much higher than highest list, or the list is
	   empty, we need new entry */
	if ( (sp == NULL) || (num > (sp->hi + 1)))
	{
		if (NULL == (tsp = (struct sub_ent *)
			malloc (sizeof (struct sub_ent)))) OutOfMemory();

		tsp->lo = tsp->hi = num;
		tsp->next = sp;

		return (tsp);
	}

	/* lsp remembers last entry in case we need to add a new entry */
	lsp = NULL;

	/* Find appropriate entry for this number */
	while (sp != NULL)
	{
		/* Have to squeeze one in before this one? */
		if (num > (sp->hi + 1))
		{
			if (NULL == (tsp = (struct sub_ent *)
				malloc (sizeof (struct sub_ent))))
					OutOfMemory();

			tsp->lo = tsp->hi = num;
			tsp->next = sp;
			lsp->next = tsp;

			return (sp_head);
		}

		/* One greater than entry's hi? */
		if (num == (sp->hi + 1))
		{
			sp->hi = num;
			return (sp_head);
		}

		/* In middle of range, do nothing */
		if ( (num >= sp->lo) && (num <= sp->hi) ) return (sp_head);

		/* One too lo, must check if we merge with next entry */
		if (num == (sp->lo - 1))
		{
			if (NULL == sp->next)
			{
				/* No next entry to merge with */
				sp->lo = num;
				return (sp_head);
			}

			/* Check for merge */
			if (num == (sp->next->hi + 1))
			{
				tsp = sp->next;
				sp->lo = tsp->lo;
				sp->next = tsp->next;
				free (tsp);
				return (sp_head);
			}
			else
			{
				/* No merge */
				sp->lo = num;
				return (sp_head);
			}
		}

		lsp = sp;
		sp = sp->next;
	}

	/* We flew off the end and need a new entry */
	if (NULL == (tsp = (struct sub_ent *)
		malloc (sizeof (struct sub_ent)))) OutOfMemory();

	tsp->lo = tsp->hi = num;
	tsp->next = NULL;
	lsp->next = tsp;

	return (sp_head);
}

WriteSub (fd, sp)
FILE *fd;
struct sub_ent *sp;
/*
 *  Write the subscription list
 */
{
	/* Do this recursively to reverse the order */
	ws (fd, sp, sp);
	fprintf (fd, "\n");
}

ws (fd, sp, sp_head)
FILE *fd;
struct sub_ent *sp, *sp_head;
{
	if (sp == NULL) return (0);

	/* Do the rest of them */
	ws (fd, sp->next, sp_head);

	/* Do this one */
	if (sp->lo == sp->hi)
	{
		fprintf (fd, "%d", sp->lo); fflush (fd);
	}
	else
	{
		fprintf (fd, "%d-%d", sp->lo, sp->hi); fflush (fd);
	}

	if (sp != sp_head) fprintf (fd, ",");
}

struct sub_ent *FixSub (sp, lo, hi)
struct sub_ent *sp;
int lo, hi;
/*
 *  Sanity fixes to the subscription list
 */
{
	struct sub_ent *tsp1, *tsp2, *tsp3;

	/* If the list is empty, make one new entry marking everything
	   up to the lowest available article as read */
	if (sp == NULL)
	{
		if (NULL == (tsp1 = (struct sub_ent *) malloc
			(sizeof (struct sub_ent)))) OutOfMemory();

		tsp1->lo = 1;
		tsp1->hi = (lo > 1) ? (lo-1) : 1;
		tsp1->next = NULL;
		return (tsp1);
	}

	/* If the highest read article is greater than the highest
	   available article, assume the group has been reset */
	if (sp->hi > hi)
	{
		/* Mark everything as unread */
		sp->lo = 1;
		sp->hi = (lo > 1) ? (lo-1) : 1;

		/* Free the rest */
		tsp1 = sp->next;
		while (tsp1 != NULL)
		{
			tsp2 = tsp1->next;
			free (tsp1);
			tsp1 = tsp2;
		}
		sp->next = NULL;
		return (sp);
	}

	/* Now walk through the list and eliminate ranges lower
	   than the lowest available article */
	tsp1 = sp;
	while (tsp1 != NULL)
	{
		/* If lowest read article of this range is smaller
		   than the lowest available article, all the rest
		   of the ranges are unnecessary */

		if (tsp1->lo < lo)
		{
			/* Make this range from 1 to lo */
			tsp1->lo = 1;
			if (tsp1->hi < lo) tsp1->hi = lo - 1;

			/* Free the rest */
			tsp2 = tsp1->next;
			while (tsp2 != NULL)
			{
				tsp3 = tsp2->next;
				free (tsp2);
				tsp2 = tsp3;
			}
			tsp1->next = NULL;
			return (sp);
		}
		tsp1 = tsp1->next;
	}
	return (sp);	/* Probably shouldn't get here */
}

int OpenZipFiles ()
/*
 *  Open files for a ZipNews news packet
 */
{
	char fn[PATH_LEN];

	/* Open .nws file */
	sprintf (fn, "%s/%s.nws", home_dir, user_name);
	if (NULL == (nws_fd = fopen (fn, "wb")))
	{
		fprintf (stderr, "%s: can't open %s\n", progname, fn);
		return (0);
	}

	/* Open .idx file */
	sprintf (fn, "%s/%s.idx", home_dir, user_name);
	if (NULL == (idx_fd = fopen (fn, "wb")))
	{
		fprintf (stderr, "%s: can't open %s\n", progname, fn);
		fclose (nws_fd);
		return (0);
	}

	return (1);
}

WriteJoin ()
/*
 *  Write the ZipNews join file
 */
{
	FILE *fd;
	char fn[PATH_LEN];
	struct nrc_ent *np;

	/* Open join file */
	sprintf (fn, "%s/%s.jn", home_dir, user_name);
	if (NULL == (fd = fopen (fn, "wb")))
	{
		fprintf (stderr, "%s: can't open %s\n", progname, fn);
		return (0);
	}

	/* Walk through the newsrc */
	np = nrc_list;
	while (np != NULL)
	{
		if (np->subscribed)
		{
			if (np->sub == NULL)
			{
				fprintf (fd, "%s 0\r\n", np->name);
			}
			else
			{
				fprintf (fd, "%s %d\r\n",
					np->name, np->sub->hi);
			}
		}
		np = np->next;
	}

	fclose (fd);
	return (1);
}

NdxClose (fd)
FILE *fd;
/*
 *  Close ndx file, remove if empty
 */
{
	long offset;

	/* See how big it is */
	offset = ftell (fd);

	/* Close it */
	fclose (fd);

	/* Remove it if it's empty */
	if (offset == 0) unlink (ndx_fn);
}

MsgClose (fd)
FILE *fd;
/*
 *  Close msg file, remove if empty
 */
{
	long offset;

	/* See how big it is */
	offset = ftell (fd);

	/* Close it */
	fclose (fd);

	/* Remove it if it's empty */
	if (offset == 0) unlink (msg_fn);
}

DoXref (fd, bytes)
FILE *fd;
long bytes;
/*
 *  Look for Xref line.
 */
{
	long offset;
	int n;
	char *rc;

	/* Remember file position */
	offset = ftell (fd);

	/* Look through header */
	rc = Fgets (buf, BUF_LEN, fd);
	n = strlen (buf);
	while ( (rc != NULL) && (bytes > 0) && (n > 0) )
	{
		/* Xref: line? */
		if (!strncmp (buf, "Xref: ", 6))
		{
			/* Found one, process it */
			Xref (buf);

			/* Restore position, return */
			fseek (fd, offset, 0);
			return (1);
		}

		/* Get next line */
		bytes -= n;
		rc = Fgets (buf, BUF_LEN, fd);
		if (rc != NULL) n = strlen (buf);
	}	

	/* Reposition file */
	fseek (fd, offset, 0);
	return (0);
}

Xref (s)
char *s;
/*
 *  Process an Xref line.
 */
{
	char *c, *p, name[PATH_LEN];
	int num;
	struct nrc_ent *np;

	/* Skip the first two fields (Xref: and host) */
	c = strtok (s, " \t");
	if (c == NULL) return (0);
	c = strtok (NULL, " \t");
	if (c == NULL) return (0);

	/* Look through the rest of the fields */
	c = strtok (NULL, " \t");
	while (c != NULL)
	{
		/* Change : to space */
		for (p=c; *p; p++) if (*p == ':') *p = ' ';

		/* Parse xref entry */
		if (2 == sscanf (c, "%s %d", name, &num))
		{
			/* Find nrc entry for this group */
			for (np=nrc_list; np!=NULL; np=np->next)
			{
				/* Match? */
				if (!strcmp (np->name, name))
				{
					/* Mark as read */
					np->sub = MarkRead (num, np->sub);
					break;
				}
			}
		}

		/* Next xref entry */
		c = strtok (NULL, " \t");
	}

	return (0);
}

SumArticle (fd, artnum, bytes, np)
FILE *fd;
long bytes;
struct nrc_ent *np;
int artnum;
/*
 *  Summarize article
 */
{
	char subject[PATH_LEN];

	/* Get subject line */
	if (!GetHdr (fd, subject, bytes, "Subject: ")) return (0);

	/* Write group name if this is first article for group */
	if (sum_flag == 0)
	{
		sum_flag = 1;
		fprintf (sum_fd, "\r\n*** %s\r\n", np->name);
	}

	/* Write article number, subject line */
	fprintf (sum_fd, "%d:%s\r\n", artnum, subject);
}

DoSelect ()
/*
 *  Select articles from a summary file
 */
{
	char group[PATH_LEN], news_path[PATH_LEN], *p;
	int artnum;
	struct act_ent *ap;
	struct nrc_ent *np, *tnp;
	struct conf_ent *cp;

	/* Open selection file */
	if (NULL == (sel_fd = fopen (sel_file, "r")))
	{
		fprintf (stderr, "%s: can't open %s\n", progname, sel_file);
		return (0);
	}

	/* Show no group yet */
	group[0] = 0;

	/* Read through lines */
	while (NULL != Fgets (buf, BUF_LEN, sel_fd))
	{
		/* Check if too many blocks already */
		if ( (blk_cnt >= max_blks) && (max_blks > 0) ) break;

		/* Ignore empty lines */
		if (0 == strlen (buf)) continue;

		/* First character determines type of line */
		switch (buf[0])
		{
		case '*':	/* New group */

			/* Close old index files if any */
			if (group[0] != 0)
			{
				if (!slnp_mode && !zip_mode && !sum_mode)
							NdxClose (ndx_fd);
				if (slnp_mode) MsgClose (msg_fd);
			}

			/* New newsgroup */
			if (1 != sscanf (buf, "%*s %s", group))
			{
				group[0] = 0;
				break;
			}

			/* Find it in newsrc, bail if not there */
			np = NULL;
			for (tnp=nrc_list; tnp!=NULL; tnp=tnp->next)
			{
				if (!strcmp (tnp->name, group)) np = tnp;
			}
			if (np == NULL)
			{
				group[0] = 0;
				break;
			}

			printf ("%s: %s\n", progname, group);

			/* Make new conference */
			cp = NewConference (group, np->conf);

			/* No ZipNews index yet */
			if (zip_mode) zip_flag = 0;
#ifdef SERVER
			/* Select group from NNTP server */
			group_nntp (group);
#else
			/* Make news path */
			sprintf (news_path, "%s/%s/", news_dir, group);
			for (p=(&news_path[0]); *p; p++)
				if (*p == '.') *p = '/';
#endif
			/* Look up in active file */
			ap = FindActive (group);

			/* Do nothing if not in active file */
			if (ap == NULL)
			{
				/* Show no valid group */
				group[0] = 0;

				if (!slnp_mode && !zip_mode && !sum_mode)
							NdxClose (ndx_fd);
				if (slnp_mode) MsgClose (msg_fd);
			}
			else
			{
				/* Fix up the subscription list */
				np->sub = FixSub (np->sub, ap->lo, ap->hi);
			}

			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			/* Wants article; skip if no group */
			if (group[0] != 0)
			{
				/* Get article number */
				sscanf (buf, "%d", &artnum);

				/* Process it */
				DoArticle (news_path, artnum, np, cp);

				/* Mark as read */
				np->sub = MarkRead (artnum, np->sub);
			}
			break;

		default:
			break;
		}
	}

	fclose (sel_fd);
	return (1);
}
