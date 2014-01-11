/*
 *  Header for uqwk
 */

#define UQWK_VERSION	"1.8 (OS/2)"

#ifdef ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

#define TEMPDIR     (getenv("TEMP") ? getenv("TEMP") : ".")

#define	PATH_LEN	(256)	/* Length for file names, etc. */
#define BUF_LEN		(4096)	/* Length of general purpose buffer */

#define QWK_EOL		(227)	/* QWK end-of-line */
#define QWK_ACT_FLAG	(225)	/* Active message flag */
#define QWK_PUBLIC	' '	/* Public message flags */
#define QWK_PUBLIC2	'-'
#define QWK_PRIVATE	'+'	/* Private message flags */
#define QWK_PRIVATE2	'*'

#define MAILER_PATH	"mail"	/* Where to find mailer */
#define INEWS_PATH      (getenv("POSTER") ? getenv("POSTER") : "inews")  /* Where to find inews */
#define SENDMAIL_PATH   (getenv("MAILER") ? getenv("MAILER") : "sendmail")

#define MAIL_CONF_NAME  "Email"

/* Defaults */
#define	DEF_MAIL_DIR	"/usr/spool/mail"
#define DEF_DO_MAIL	(1)
#define DEF_DO_NEWS	(0)
#define DEF_INC_HDRS	(1)
#define DEF_PRT_OPTS	(0)
#define DEF_READ_ONLY	(0)
#define DEF_WAF_MODE	(0)
#define DEF_SLNP_MODE	(0)
#define DEF_ZIP_MODE	(0)
#define DEF_XRF_MODE	(0)
#define DEF_BW_KLUDGE	(0)
#define DEF_XPRT_MODE	(0)
#define DEF_SUM_MODE	(0)
#define DEF_SEL_MODE	(0)
#define DEF_EVERY_MODE	(0)
#define DEF_MAX_BLKS	(4000)	/* That's half a megabyte */
#define DEF_GRP_LEN	(15)
#define DEF_HOME_DIR	(getenv("HOME") ? getenv("HOME") : ".")
#define DEF_USER_NAME	(getenv("USER") ? getenv("USER") : "unknown")
#define DEF_MAIL_FILE	"unknown"
#define DEF_BBS_NAME	"unknown BBS"
#define DEF_BBS_CITY	"Anytown, USA"
#define DEF_BBS_PHONE	"555-1212"
#define DEF_BBS_SYSOP	"Joe Sysop"
#define DEF_BBS_ID	"0,SOMEBBS"
#define DEF_ACT_FILE	"/usr/lib/news/active"
#define DEF_NRC_FILE	"unknown"
#define DEF_NEWS_DIR	"/usr/spool/news"
#define DEF_REP_FILE	"none"
#define DEF_TRN_FILE	"none"
#define DEF_HOST_NAME	"nowhere"
#define DEF_SUM_FILE	"none"
#define DEF_SEL_FILE	"none"
#define DEF_NG_FILE	"none"

/* Runtime options */
EXTERN	int do_mail;			/* Process mail? */
EXTERN	int do_news;			/* Process news? */
EXTERN	int inc_hdrs;			/* Include headers in messages? */
EXTERN	int prt_opts;			/* Display options; no processing */
EXTERN	int read_only;			/* Don't rewrite mail and .newsrc */
EXTERN	int max_blks;			/* Maximum blocks per QWK packet */
EXTERN	int grp_len;			/* Maximum newsgroup name length */
EXTERN	int waf_mode;			/* Waffle type .newsrc */
EXTERN	int slnp_mode;			/* Write SLNP packet */
EXTERN	int zip_mode;			/* Write ZipNews packet */
EXTERN	int xrf_mode;			/* Cancel crosspostings */
EXTERN	int bw_kludge;			/* Blue Wave kludge */
EXTERN	int xprt_mode;			/* Expert mode */
EXTERN	int sum_mode;			/* Summary mode */
EXTERN	int sel_mode;			/* Selection mode */
EXTERN	int every_mode;			/* Write every grp to CONTROL.DAT */

EXTERN	char mail_file[PATH_LEN];	/* mail spool */
EXTERN	char mail_dir[PATH_LEN];	/* dir for mail spool */
EXTERN	char home_dir[PATH_LEN];	/* home directory */
EXTERN	char user_name[PATH_LEN];	/* user's login name */
EXTERN	char bbs_name[PATH_LEN];	/* BBS name */
EXTERN	char bbs_city[PATH_LEN];	/* BBS city */
EXTERN	char bbs_phone[PATH_LEN];	/* BBS phone number */
EXTERN	char bbs_sysop[PATH_LEN];	/* BBS sysop name */
EXTERN	char bbs_id[PATH_LEN];		/* BBS ID */
EXTERN	char act_file[PATH_LEN];	/* Active file */
EXTERN	char nrc_file[PATH_LEN];	/* .newsrc file */
EXTERN	char news_dir[PATH_LEN];	/* News spool dir */
EXTERN	char rep_file[PATH_LEN];	/* Reply packet file name */
EXTERN	char trn_file[PATH_LEN];	/* Newsgroup name translation table */
EXTERN	char host_name[PATH_LEN];	/* Host machine name */
EXTERN	char sum_file[PATH_LEN];	/* Summary file */
EXTERN	char sel_file[PATH_LEN];	/* Selection file */
EXTERN	char ng_file[PATH_LEN];		/* Desired ng file */

char *getenv();
char *Fgets();
char *ParseFrom();
struct act_ent *FindActive();
struct sub_ent *SubList();
struct sub_ent *MarkRead();
struct sub_ent *FixSub();
struct conf_ent *NewConference();

/* Various globals */
EXTERN	char *progname;			/* Program name */
EXTERN	int msg_cnt;			/* Total number of messages */
EXTERN	int conf_cnt;			/* Total number of conferences */
EXTERN	FILE *msg_fd;			/* MESSAGES.DAT file desc */
EXTERN	char msg_fn[PATH_LEN];		/* SLNP *.MSG file name */
EXTERN	FILE *ctl_fd;			/* CONTROL.DAT file desc */
EXTERN	FILE *ndx_fd;			/* xxx.NDX file desc */
EXTERN	char ndx_fn[PATH_LEN];		/* xxx.NDX file name */
EXTERN	FILE *act_fd;			/* Active file file desc */
EXTERN	FILE *nrc_fd;			/* .newsrc file desc */
EXTERN	FILE *rep_fd;			/* Reply packet file desc */
EXTERN	FILE *nws_fd;			/* ZipNews .nws file desc */
EXTERN	FILE *idx_fd;			/* ZipNews .idx file desc */
EXTERN	FILE *mai_fd;			/* ZipNews .mai file desc */
EXTERN	int zip_flag;			/* idx entry written for this ng? */
EXTERN	unsigned char buf[BUF_LEN];	/* General purpose buffer */
EXTERN	int blk_cnt;			/* Blocks written to messages.dat */
EXTERN	int sum_flag;			/* Summary hdr for this ng yet? */
EXTERN	FILE *sum_fd;			/* Summary file desc */
EXTERN	FILE *sel_fd;			/* Selection file desc */

/* This is the stuff we remember about each spooled mail message */
EXTERN	struct mail_ent
{
	long begin;		/* Offset of start of header */
	long text;		/* Offset to end of header, start of text */
	long end;		/* Offset to start of next message */
	struct mail_ent *next;	/* Pointer to next */
} *mail_list;

/* This is stuff we remember about each "conference" */
EXTERN	struct conf_ent
{
	char *name;		/* Conference name */
	int number;		/* Conference number */
	int count;		/* Number of articles in this conference */
	struct conf_ent *next;	/* Pointer to next */
} *conf_list, *last_conf;

/* This is the QWK message header format */
struct qwk_hdr
{
	unsigned char status;
	unsigned char number[7];
	unsigned char date[8];
	unsigned char time[5];
	unsigned char to[25];
	unsigned char from[25];
	unsigned char subject[25];
	unsigned char password[12];
	unsigned char refer[8];
	unsigned char blocks[6];
	unsigned char flag;
	unsigned char conference[2];
	unsigned char msg_num[2];
	unsigned char tag;
};

EXTERN	struct qwk_hdr rep_hdr;		/* Header for replies */

/* Stuff we remember about each active newsgroup */
EXTERN	struct act_ent
{
	char *name;		/* Newsgroup name */
	int hi;			/* High article number */
	int lo;			/* Low article number */
	struct act_ent *next;	/* Pointer to next */
} *act_list;

/* Stuff we remember about the .newsrc file */
EXTERN	struct nrc_ent
{
	char *name;		/* Newsgroup name */
	int subscribed;		/* Subscribed flag */
	struct sub_ent *sub;	/* Subscription list */
	int conf;		/* Corresponding conference number */
	struct nrc_ent *next;	/* Pointer to next */
} *nrc_list;

/* Stuff we remember about the newsgroup name translation table */
EXTERN	struct trn_ent
{
	char *old;		/* Old name */
	char *new;		/* New name */
	struct trn_ent *next;	/* Pointer to next */
} *trn_list;

/* Stuff we remember about a subscription list (in the .newsrc) */
struct sub_ent
{
	int lo, hi;		/* Range of articles */
	struct sub_ent *next;	/* Pointer to next */
};

/* Stuff we remember about the desired newsgroup list */
EXTERN	struct ng_ent
{
	char *name;		/* Newsgroup name */
	struct ng_ent *next;	/* Pointer to next */
} *ng_list;
