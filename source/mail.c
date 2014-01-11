#include <stdio.h>
#include "uqwk.h"

/*
 *  All sorts of stuff to do mail processing
 */

FILE *mail_fd;			/* For mail spool */
struct mail_ent *last_mp;	/* Points to last mail list entry */

DoMail ()
/*
 *  Process mail into QWK packet
 */
{
	struct mail_ent *mailp;
	struct conf_ent *cp;

	/* No mail in summary mode */
	if (sum_mode) return (0);

	/* Open the mail spool */
	if (NULL == (mail_fd = fopen (mail_file, "rb")))
	{
		fprintf (stderr, "%s: can't open %s\n", progname, mail_file);
		perror (progname);
		return (0);
	}

	/* Define the mail "conference" */
	cp = NewConference (MAIL_CONF_NAME, conf_cnt);

	/* Construct the mail linked list */
	MakeMailList ();

	/* Open ZipNews mail file */
	if (zip_mode && (mail_list != NULL)) OpenZipMail();

	/* Walk through all the messages */
	mailp = mail_list;

	while (mailp != NULL)
	{
		cp->count++;

		if (slnp_mode)
		{
			SLNPMessage (mailp);
		}
		else if (zip_mode)
		{
			ZipMessage (mailp);
		}
		else
		{
			DoMessage (mailp);
		}

		mailp = mailp->next;
	}

	fclose (mail_fd);
	if (!slnp_mode && !zip_mode) fclose (ndx_fd);
	if (slnp_mode) fclose (msg_fd);
	if (zip_mode && (mail_list != NULL)) fclose (mai_fd);

	/* Now empty the mail box */
	if (!read_only)
	{
		if (NULL == (mail_fd = fopen (mail_file, "wb")))
		{
			fprintf (stderr, "%s: can't write %s\n", progname,
								mail_file);
		}
		else
		{
			fclose (mail_fd);
		}
	}
	return (1);
}

MakeMailList ()
/*
 *  Construct linked list of pointers to individual messages in
 *  the mail spool.
 */
{
	long offset;

	last_mp = NULL;

	/* Read through, looking for "From" lines */
	offset = ftell (mail_fd);
	while (NULL != Fgets (buf, BUF_LEN, mail_fd))
	{
		if (!strncmp (buf, "From ",  5))
		{
			DoFromLine (offset);
		}
		offset = ftell (mail_fd);
	}
	if (last_mp != NULL) last_mp->end = offset;
}

DoFromLine (offset)
long offset;
{
	struct mail_ent *mp;

	/* Get space for new mail list entry */
	if (NULL==(mp=(struct mail_ent *) malloc(sizeof(struct mail_ent))))
	{
		fprintf (stderr, "%s: out of memory\n", progname);
		exit (0);
	}

	/* Fill in offset */
	mp->begin = offset;

	if (last_mp == NULL)
	{
		/* This is first message */
		mail_list = mp;
	}
	else
	{
		/* Add to end of list */
		last_mp->next = mp;
		last_mp->end = offset;
	}

	mp->next = NULL;
	last_mp = mp;
}

SLNPMessage (mp)
struct mail_ent *mp;
/*
 *  Convert a message to SLNP format
 */
{
	int bytes, c, b;

	/* Compute message size */
	b = bytes = mp->end - mp->begin;

	/* Write byte count line */
	for (c=3; c>=0; c--)
	{
		buf[c] = b % 256;
		b = b / 256;
	}
	fwrite (buf, 4, 1, msg_fd);

	/* Seek to start of message */
	fseek (mail_fd, mp->begin, 0);

	/* Copy bytes */
	while (bytes--)
	{
		c = fgetc (mail_fd);
		fputc ((0xff & c), msg_fd);
	}
	return (0);
}

DoMessage (mp)
struct mail_ent *mp;
/*
 *  Convert a message to QWK format
 */
{
	struct qwk_hdr hdr;
	char c[PATH_LEN], *eof, ndx[5];
	int out_bytes, n, i;

	/* Write the ndx file entry */
	inttoms (blk_cnt, ndx);
	ndx[4] = conf_cnt;
	fwrite (ndx, 5, 1, ndx_fd);

	Spaces (&hdr, 128);

	/* Fill in the header fields we can do now */
	hdr.status = QWK_PRIVATE;
	PadNum (msg_cnt, hdr.number, 7);
	Spaces (hdr.password, 12);
	Spaces (hdr.refer, 8);
	hdr.flag = QWK_ACT_FLAG;
	IntNum (conf_cnt, hdr.conference);
	IntNum (msg_cnt+1, hdr.msg_num);
	hdr.tag = ' ';

	msg_cnt++;

	/* Seek to start of message */
	fseek (mail_fd, mp->begin, 0);

	/* Read the From line */
	Fgets (buf, BUF_LEN, mail_fd);

	/* The second field of the From line is used as a first
	   guess for who sent the message */
	sscanf (&buf[5], "%s", c);
	PadString (c, hdr.from, 25);

	/* Now read through header lines, looking for ones we need */
	eof = Fgets (buf, BUF_LEN, mail_fd);
	while ( (0 != strlen(buf)) && (eof != NULL) )
	{
		if (!strncmp (buf, "Date: ", 6))
		{
			ParseDate (&buf[6], &hdr);
		}
		else if (!strncmp (buf, "To: ", 4))
		{
			PadString (&buf[4], hdr.to, 25);
		}
		else if (!strncmp (buf, "Subject: ", 9))
		{
			PadString (&buf[9], hdr.subject, 25);
		}
		else if (!strncmp (buf, "From: ", 6))
		{
			PadString (ParseFrom(&buf[6]), hdr.from, 25);
		}

		eof = Fgets (buf, BUF_LEN, mail_fd);
	}
	mp->text = ftell (mail_fd);

	/* Fill in block count */
	if (inc_hdrs)
	{
		PadNum (2+(mp->end-mp->begin)/128, hdr.blocks, 6);
		blk_cnt += (1+(mp->end - mp->begin)/128);
	}
	else
	{
		PadNum (2+(mp->end-mp->text)/128, hdr.blocks, 6);
		blk_cnt += (1+(mp->end - mp->text)/128);
	}

	/* Write out the message header */
	fwrite (&hdr, 128, 1, msg_fd);
	blk_cnt++;

	/* Now write the message text */
	if (inc_hdrs) fseek (mail_fd, mp->begin, 0);
	out_bytes = 0;

	eof = Fgets (buf, BUF_LEN, mail_fd);
	do
	{
		n = strlen (buf);

		/* MMDF puts funny things in messages -- change to spaces */
		for (i=0; i<n; i++)
		{
			if (buf[i] == 1) buf[i] = ' ';
			if (buf[i] == 0) buf[i] = ' ';
		}

		fwrite (buf, n, 1, msg_fd);
		out_bytes += n;
		if (n < BUF_LEN-1)
		{
			fputc (QWK_EOL, msg_fd);
			out_bytes++;
		}
		eof = Fgets (buf, BUF_LEN, mail_fd);
	} while ( (strncmp(buf,"From ", 5)) && (NULL != eof) );

	/* Pad block as necessary */
	n = out_bytes % 128;
	for (;n<128;n++) fputc (' ', msg_fd);
}

OpenZipMail ()
/*
 *  Open ZipNews mail file
 */
{
	char fn[PATH_LEN];

	/* Make name */
	sprintf (fn, "%s/%s.mai", home_dir, user_name);

	/* Open it */
	if (NULL == (mai_fd = fopen (fn, "wb")))
	{
		fprintf (stderr, "%s: can't open %s\n", progname, fn);
		exit (0);
	}
}

ZipMessage (mp)
struct mail_ent *mp;
/*
 *  Convert a message to Zip format
 */
{
	int bytes, c;

	/* Compute message size */
	bytes = mp->end - mp->begin;

	/* Write separator */
	for (c=0; c<20; c++) fputc (1, mai_fd);
	fprintf (mai_fd, "\r\n");

	/* Seek to start of message */
	fseek (mail_fd, mp->begin, 0);

	/* Copy bytes */
	while (bytes--)
	{
		c = fgetc (mail_fd);

		/* ZipNews doesn't like ^Z's */
		if (c == 26) c = 32;

		/* Map LF to CRLF */
		if (c == 10) fputc (13, mai_fd);

		fputc ((0xff & c), mai_fd);
	}
	return (0);
}
