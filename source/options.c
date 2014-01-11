#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include "uqwk.h"

/*
 *  Determine runtime options
 */

DefaultOptions()
/*
 *  Set up default options
 */
{
	struct passwd *pw;

	/* Do user-specific stuff*/
	if (NULL == (pw = getpwuid(getuid())))
	{
/* 		fprintf (stderr, "%s: warning: you don't exist\n", progname); */
		strcpy (user_name, DEF_USER_NAME);
		strcpy (home_dir, DEF_HOME_DIR);
	}
	else
	{
		strcpy (user_name, pw->pw_name);
		strcpy (home_dir, pw->pw_dir);
	}

	/* Dinky misc options */
	do_mail = DEF_DO_MAIL;
	do_news = DEF_DO_NEWS;
	inc_hdrs = DEF_INC_HDRS;
	prt_opts = DEF_PRT_OPTS;
	read_only = DEF_READ_ONLY;
	max_blks = DEF_MAX_BLKS;
	grp_len = DEF_GRP_LEN;
	waf_mode = DEF_WAF_MODE;
	slnp_mode = DEF_SLNP_MODE;
	zip_mode = DEF_ZIP_MODE;
	xrf_mode = DEF_XRF_MODE;
	bw_kludge = DEF_BW_KLUDGE;
	xprt_mode = DEF_XPRT_MODE;
	sum_mode = DEF_SUM_MODE;
	sel_mode = DEF_SEL_MODE;
	every_mode = DEF_EVERY_MODE;

	strcpy (mail_dir, DEF_MAIL_DIR);
	strcpy (mail_file, DEF_MAIL_FILE);
	strcpy (act_file, DEF_ACT_FILE);
	strcpy (nrc_file, DEF_NRC_FILE);
	strcpy (news_dir, DEF_NEWS_DIR);

	strcpy (bbs_name, DEF_BBS_NAME);
	strcpy (bbs_city, DEF_BBS_CITY);
	strcpy (bbs_phone, DEF_BBS_PHONE);
	strcpy (bbs_sysop, DEF_BBS_SYSOP);
	strcpy (bbs_id, DEF_BBS_ID);
	strcpy (rep_file, DEF_REP_FILE);
	strcpy (trn_file, DEF_TRN_FILE);
	strcpy (host_name, DEF_HOST_NAME);
	strcpy (sum_file, DEF_SUM_FILE);
	strcpy (sel_file, DEF_SEL_FILE);
	strcpy (ng_file, DEF_NG_FILE);
}

EnvOptions()
/*
 *  Override options from environment variables
 */
{
	char *c;

	if (NULL != (c = getenv ("UQ_DO_MAIL"))) do_mail = atoi (c);
	if (NULL != (c = getenv ("UQ_DO_NEWS"))) do_news = atoi (c);
	if (NULL != (c = getenv ("UQ_INC_HDRS"))) inc_hdrs = atoi (c);
	if (NULL != (c = getenv ("UQ_PRT_OPTS"))) prt_opts = atoi (c);
	if (NULL != (c = getenv ("UQ_READ_ONLY"))) read_only = atoi (c);
	if (NULL != (c = getenv ("UQ_MAX_BLKS"))) max_blks = atoi (c);
	if (NULL != (c = getenv ("UQ_GRP_LEN"))) grp_len = atoi (c);
	if (NULL != (c = getenv ("UQ_WAF_MODE"))) waf_mode = atoi (c);
	if (NULL != (c = getenv ("UQ_SOUP_MODE"))) slnp_mode = atoi (c);
	if (NULL != (c = getenv ("UQ_ZIP_MODE"))) zip_mode = atoi (c);
	if (NULL != (c = getenv ("UQ_XRF_MODE"))) xrf_mode = atoi (c);
	if (NULL != (c = getenv ("UQ_BW_KLUDGE"))) bw_kludge = atoi (c);
	if (NULL != (c = getenv ("UQ_XPRT_MODE"))) xprt_mode = atoi (c);
	if (NULL != (c = getenv ("UQ_EVERY_MODE"))) every_mode = atoi (c);

	if (NULL != (c = getenv ("UQ_HOME_DIR"))) strcpy (home_dir, c);
	if (NULL != (c = getenv ("UQ_MAIL_FILE"))) strcpy (mail_file, c);
	if (NULL != (c = getenv ("UQ_MAIL_DIR"))) strcpy (mail_dir, c);
	if (NULL != (c = getenv ("UQ_USER_NAME"))) strcpy (user_name, c);
	if (NULL != (c = getenv ("UQ_NEWS_DIR"))) strcpy (news_dir, c);

	if (NULL != (c = getenv ("UQ_BBS_NAME"))) strcpy (bbs_name, c);
	if (NULL != (c = getenv ("UQ_BBS_CITY"))) strcpy (bbs_city, c);
	if (NULL != (c = getenv ("UQ_BBS_PHONE"))) strcpy (bbs_phone, c);
	if (NULL != (c = getenv ("UQ_BBS_SYSOP"))) strcpy (bbs_sysop, c);
	if (NULL != (c = getenv ("UQ_BBS_ID"))) strcpy (bbs_id, c);

	if (NULL != (c = getenv ("UQ_ACT_FILE"))) strcpy (act_file, c);
	if (NULL != (c = getenv ("UQ_NRC_FILE"))) strcpy (nrc_file, c);

	if (NULL != (c = getenv ("UQ_REP_FILE"))) strcpy (rep_file, c);
	if (NULL != (c = getenv ("UQ_TRN_FILE"))) strcpy (trn_file, c);
	if (NULL != (c = getenv ("UQ_HOST_NAME"))) strcpy (host_name, c);
	if (NULL != (c = getenv ("UQ_NG_FILE"))) strcpy (ng_file, c);

	if (NULL != (c = getenv ("UQ_SUM_FILE")))
	{
		strcpy (sum_file, c);
		sum_mode = 1;
	}
	if (NULL != (c = getenv ("UQ_SEL_FILE")))
	{
		strcpy (sel_file, c);
		sel_mode = 1;
	}
}

CommandOptions (argc, argv)
int argc;
char *argv[];
/*
 *  Override options from command line
 */
{
	int i;

	for (i=1; i<argc; i++)
	{
		switch (argv[i][0])
		{
		case '+':
			switch (argv[i][1])
			{
			case 'm':	do_mail = 1;
					break;

			case 'n':	do_news = 1;
					break;

			case 'h':	inc_hdrs = 1;
					break;

			case 'r':	read_only = 1;
					break;

			case 'w':	waf_mode = 1;
					break;

			case 'L':	slnp_mode = 1;
					break;

			case 'z':	zip_mode = 1;
					break;

			case 'x':	xrf_mode = 1;
					break;

			case 'W':	bw_kludge = 1;
					break;

			case 'X':	xprt_mode = 1;
					break;

			case 'e':	every_mode = 1;
					break;

			default:	BadFlag (argv[i]);
					break;
			}
			break;

		case '-':
			switch (argv[i][1])
			{
			case 'm':	do_mail = 0;
					break;

			case 'n':	do_news = 0;
					break;

			case 'h':	inc_hdrs = 0;
					break;

			case 'r':	read_only = 0;
					break;

			case 'w':	waf_mode = 0;
					break;

			case 'p':	prt_opts = 1;
					break;

			case 'L':	slnp_mode = 0;
					break;

			case 'z':	zip_mode = 0;
					break;

			case 'x':	xrf_mode = 0;
					break;

			case 'W':	bw_kludge = 0;
					break;

			case 'X':	xprt_mode = 0;
					break;

			case 'e':	every_mode = 0;
					break;

			case 'M':	strcpy (mail_dir, &argv[i][2]);
					break;

			case 'f':	strcpy (mail_file, &argv[i][2]);
					break;

			case 'u':	strcpy (user_name, &argv[i][2]);
					break;

			case 'H':	strcpy (home_dir, &argv[i][2]);
					break;

			case 'b':	strcpy (bbs_name, &argv[i][2]);
					break;

			case 'c':	strcpy (bbs_city, &argv[i][2]);
					break;

			case 'P':	strcpy (bbs_phone, &argv[i][2]);
					break;

			case 's':	strcpy (bbs_sysop, &argv[i][2]);
					break;

			case 'i':	strcpy (bbs_id, &argv[i][2]);
					break;

			case 'a':	strcpy (act_file, &argv[i][2]);
					break;

			case 'N':	strcpy (nrc_file, &argv[i][2]);
					break;

			case 'S':	strcpy (news_dir, &argv[i][2]);
					break;

			case 'B':	max_blks = atoi (&argv[i][2]);
					break;

			case 'R':	strcpy (rep_file, &argv[i][2]);
					break;

			case 'l':	grp_len = atoi (&argv[i][2]);
					break;

			case 't':	strcpy (trn_file, &argv[i][2]);
					break;

			case 'd':	strcpy (host_name, &argv[i][2]);
					break;

			case 'U':	strcpy (sum_file, &argv[i][2]);
					sum_mode = 1;
					break;

			case 'E':	strcpy (sel_file, &argv[i][2]);
					sel_mode = 1;
					break;

			case 'D':	strcpy (ng_file, &argv[i][2]);
					break;

			default:	BadFlag (argv[i]);
					break;
			}
			break;

		default:
			BadFlag (argv[i]);
			break;
		}
	}

	/* If mail file has not been overridden, set it */
	if (!strcmp (mail_file, DEF_MAIL_FILE))
	{
		strcpy (mail_file, mail_dir);
		strcat (mail_file, "/");
		strcat (mail_file, user_name);
	}

	/* If .newsrc file has not been overridden, set it */
	if (!strcmp (nrc_file, DEF_NRC_FILE))
	{
		strcpy (nrc_file, home_dir);
		strcat (nrc_file, "/.newsrc");
	}

	/* Some sanity checks */

	/* Summary or selection mode implies do news */
	if (sum_mode || sel_mode) do_news = 1;

	/* Summary mode implies no mail */
	if (sum_mode) do_mail = 0;

	/* Summary mode implies no special packet format */
	if (sum_mode) zip_mode = slnp_mode = 0;

	if (prt_opts)
	{
		PrintOptions();
		exit (0);
	}
}

BadFlag (c)
char *c;
{
	fprintf (stderr, "%s: bad flag: %s\n", progname, c);
	exit (0);
}

PrintOptions ()
{
	printf ("Version: %s\n", UQWK_VERSION);
	printf ("Do mail: %d\n", do_mail);
	printf ("Do news: %d\n", do_news);
	printf ("Include headers: %d\n", inc_hdrs);
	printf ("Read only: %d\n", read_only);
	printf ("Maximum blocks: %d\n", max_blks);
	printf ("Group name limit: %d\n", grp_len);
	printf ("Waffle mode: %d\n", waf_mode);
	printf ("SOUP mode: %d\n", slnp_mode);
	printf ("ZipNews mode: %d\n", zip_mode);
	printf ("Xref mode: %d\n", xrf_mode);
	printf ("Blue Wave kludge: %d\n", bw_kludge);
	printf ("Expert mode: %d\n", xprt_mode);
	printf ("Every group in CONTROL.DAT: %d\n", every_mode);
	printf ("Mail directory: %s\n", mail_dir);
	printf ("News directory: %s\n", news_dir);
	printf ("Mail file: %s\n", mail_file);
	printf ("User name: %s\n", user_name);
	printf ("Home directory: %s\n", home_dir);
	printf ("BBS name: %s\n", bbs_name);
	printf ("BBS city: %s\n", bbs_city);
	printf ("BBS phone: %s\n", bbs_phone);
	printf ("BBS sysop: %s\n", bbs_sysop);
	printf ("BBS id: %s\n", bbs_id);
	printf ("Active file: %s\n", act_file);
	printf (".newsrc file: %s\n", nrc_file);
	printf ("Reply file: %s\n", rep_file);
	printf ("Newsgroup name translation table: %s\n", trn_file);
	printf ("Host name: %s\n", host_name);
	printf ("Summary file: %s\n", sum_file);
	printf ("Selection file: %s\n", sel_file);
	printf ("Desired newsgroups file: %s\n", ng_file);
}
