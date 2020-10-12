/*
 * bbs.h - Defines specific to the BBS.
 */


/* Configuration constants */
#define BBSNAME	     "ISCABBS"
#define BBSUID       1001               /* UID of owner of all BBS files    */
#define BBSGID       1001               /* GID of owner of all BBS files */
#define NXCONF       60			/* Number of slots in X config */
#define MAILMSGS     200		/* Number of mail messages per user */
#define MSGSPERRM    200		/* Messages per room (>= MAILMSGS!!) */
#define MAXROOMS     200		/* Number of rooms the BBS will have */
#define MAXUSERS     200                /* Maximum users online at a time */
#define MAXNEWBIES   2000               /* Maximum number of pending newbies */
#define MAXQ         1000               /* Maximum queue */
#define MM_FILELEN   (200000000)		/* size of msgmain */
#define MAXVOTES     64			/* Max number of voting items */

/* System wide file constants */

#define ROOT        "/home/bbs/"             /* root directory of all BBS stuff */

#ifdef _SSL
#define SSLCERTIFICATE	ROOT"sslprivate"
#define SSLKEYPATH	ROOT"sslprivate"
#endif


#define AIDELIST    ROOT"etc/aidelist"	/* copy of who's an aide    */
#define NEWAIDELIST    ROOT"etc/newaidelist"	/* copy of who's an aide    */
#define DESCDIR     ROOT"message/desc/" /* location of room info files */
#define ERRLOG      ROOT"etc/errlog"	/* Error log file */
#define ETC         ROOT"etc/"		/* Where lots of stuff resides */
#define FORTUNE	    "/usr/bin/fortune -a"	/* Fortune! */
#define HELPDIR     ROOT"help/"		/* Location of help files    */
#define LIMITS      ROOT"etc/limits"	/* login limits for the BBS */
#define LOCKOUT     ROOT"etc/lockout"	/* Sites that are locked out */
#define LOGFILE     ROOT"etc/log"	/* System log                */
#define MESSAGE     ROOT"message/"	/* Message directory     */
#define MSGDATA     ROOT"data/msgdata"	/* Message system (non temp) data */
#define MSGMAIN     ROOT"message/msgmain"/* Main message file */
#define MOTD	    ROOT"etc/motd"	/* Message of the day */
#define TMPDATA     ROOT"data/tmpdata"	/* Temp data (not saved to disk) */
#define USERDATA    ROOT"data/userdata"	/* User index/data */
#define VOTEFILE    ROOT"etc/votedata"	/* Voting info */
#define WHODIR      ROOT"etc/who/"	/* who knows rooms directory */
#define ISCAWHO      ROOT"etc/iscawho"	/* who knows rooms directory */
#define XMSGDATA    ROOT"data/xmsgdata" /* X message data file */

/* Miscellaneous Defs */

#define ABORT                   2
#define AIDE_RM_NBR		2
#define SYSOP_RM_NBR		6
#define BACKWARD		(-1)
#define BEL                     7	/* control-g (bell) */
#define BS			'\b'	/* back space */
#define CTRL_D                  4	/* control-d */
#define CTRL_R			18	/* control-r */
#define CTRL_U			21	/* control-u */
#define CTRL_W			23	/* control-w */
#define CTRL_X                  24	/* control-x */
#define DEL			127	/* delete character */
#define FF			255	/* == 255, begin D.O.C. msg. */
#define FMTERR                  1	/* format error in D.O.C. file */
#define FORWARD			1
#define LF			'\n'	/* line feed (new line) */
#define LOBBY_RM_NBR		0
#define MAIL_RM_NBR		1
#define MARGIN                  80
#define MAXALIAS		19	/* Max size for logon names */
#define MAXNAME                 40	/* Max size for room names */
#define MES_NORMAL	        65	/* Normal message                   */
#define MES_ANON	        66	/* "****" header                    */
#define MES_AN2		        67	/* "Anonymous" header               */
#define MES_DESC                68	/* Room description "message"       */
#define MES_SYSOP		69	/* Message from sysop		    */
#define MES_XYELL		70	/* Yell of harassing X's pasted in  */
#define MES_FM			71	/* Message from FM		    */
#define MMAP_ERR		3	/* Error mmapping temp space        */
#define MNFERR                  666	/* Message not found error          */
#define REPERR			667	/* Message repetition "error"       */
#define NL			LF
#define NOISY                   0
#define PROF_REG		0	/* Regular profile */
#define PROF_EXTRA		1	/* Show extra info on others */
#define PROF_SELF		2	/* Self profile */
#define PROF_ALL		3	/* Show all info (sysop) */
#define QUIET           1
#define REVERSE		(-1)
#define QR_INUSE	2		/* Set if in use, clear if avail    */
#define QR_PRIVATE	4		/* Set for any type of private room */
#define QR_GUESSNAME	16		/* Set if it's a guessname room     */
#define QR_PASSWORD	32		/* Set if its a passworded          */
#define QR_ANONONLY	64		/* Anonymous-Only room              */
#define QR_ANON2	128		/* Anonymous-Option room            */
#define SAVE                    0
#define SP			32	/* space character */
#define SYSOP_MSG		1	/* Last message was from sysop */
#define SYSOP_FROM_SYSOP	2	/* Flag sysop post from sysop  */
#define SYSOP_FROM_USER		4	/* Flag sysop post from user   */
#define SYSOP_FROM_FM		8	/* Flag "sysop" post from FM   */
#define TWILIGHTZONE            (-1L) /* (-1L) Zapped room (?) */
#define RODSERLING              (-2L) /* (-2L) kicked out of public  room */
#define NEWUSERFORGET           (-3L) /* (-3L) so newbies aren't overwhelmed!*/
#define TAB			9


// the below is only used in update.c, but it's in here due to being used
// in multiple functions there, unless there's a better way to do it that
// I find, it's staying in here

struct room_glob {
  char roomname[MAXNAME + 1];
  long roomnumber;
  char moderator_name[200]; //[MAXALIAS + 1];
  long moderator_number;
};



struct user
{
  long    usernum;			/* User's Citadel user num   */

  unsigned int an_www:1;		/* WWW address withheld */
  unsigned int an_site:1;		/* Connection site withheld */
  unsigned int an_mail:1;		/* Mail withheld */
  unsigned int an_phone:1;		/* Phone withheld */
  unsigned int an_location:1;		/* Location (city/state/zip) withheld*/
  unsigned int an_addr:1;		/* Address withheld */
  unsigned int an_name:1;		/* Name withheld */
  unsigned int an_all:1;		/* All withheld */

  unsigned int f_duplicate:1;		/* Has duplicate/multiple accounts */
  unsigned int f_admin:1;		/* (prog, aide) */
  unsigned int f_restricted:1;		/* (badinfo, duplicate, etc.) */
  unsigned int f_prog:1;		/* Is a programmer */
  unsigned int f_badinfo:1;		/* Marked as having bad address info */
  unsigned int f_newbie:1;		/* Is a new user */
  unsigned int f_inactive:1;		/* Marked as inactive */
  unsigned int f_deleted:1;		/* Marked for deletion */

  unsigned int f_ownnew:1;		/* See own posts as new */
  unsigned int f_namechanged:1;		/* Has had name changed */
  unsigned int f_noanon:1;		/* Not permitted to make anon posts */
  unsigned int f_noclient:1;		/* Not permitted to use BBS client */
  unsigned int f_ansi:1;		/* Wants ANSI query at login */
  unsigned int f_trouble:1;		/* New user from a "trouble" site */
  unsigned int f_invisible:1;		/* User is invisible to normal users */
  unsigned int f_autoelf:1;		/* Guide flag enabled upon login */

  unsigned int f_novice:1;		/* Is an "novice" user */
  unsigned int f_elf:1;			/* Is a guide */
  unsigned int f_nobeep:1;		/* Have X's not cause a beep */
  unsigned int f_xmsg:1;		/* Have X's arrive while busy */
  unsigned int f_ampm:1;		/* Show time in AM/PM format */
  unsigned int f_shortwho:1;		/* Show short wholist by defaut */
  unsigned int f_twit:1;		/* Is a twit */

  unsigned int f_beeps:1;		/* allow BEEPS keyword */
  unsigned int f_lastold:1;		/* Show 1 old post when reading new */
  unsigned int f_xoff:1;		/* X's off by default */
  unsigned int f_clear:1;		/* Mark posts old after any read new */
  unsigned int f_aide:1;		/* Is a sysop */
  unsigned int f_revwho:1;		/* Reverse the order of the wholist */

  int     timescalled;                  /* how many calls */
  int     posted;                       /* number of msgs posted */
  time_t  time;                         /* Time of last login        */
  time_t  timeoff;                      /* Time of last logout */
  time_t  timetot;                      /* Total time online for day */
  time_t  firstcall;			/* When user acct was created */
  char    name[MAXALIAS + 1];		/* Login name                */
  char    passwd[14];			/* the user's crypted passwd */
  char    remote[40];			/* how connected             */
  char    real_name[MAXNAME + 1];	/* User's real name          */
  char    addr1[MAXNAME + 1];		/* Home address 1 of 2       */
  char    addr2[MAXNAME + 1];
  char    city[21];			/* User's city               */
  char    state[21];			/* Users's state or country */
  char    zip[11];			/* Zipcode or mailcode        */
  char    phone[21];			/* Phone in format (AAA)-NNN-NNNN */
  char    mail[41];			/* User's internet mail address */
  char    desc1[81];			/* User description 1 of 5   */
  char    desc2[81];
  char    desc3[81];
  char    desc4[81];
  char    desc5[81];
  char    generation[MAXROOMS];		/* generation # flags joined rooms */
  char    forget[MAXROOMS];		/* generation # flags forgotten rooms */
  long    lastseen[MAXROOMS];           /* last message seen in each room */
  long	  quickx[10];			/* 10 slots for QuickX */
  struct xconf
  {
    unsigned int :7;
    unsigned int which:1;		/* 0 = disable, 1 = enable   */
    unsigned int usernum:24;		/* User to enable/disable*/
  } xconf[NXCONF];
  char www[60];
  time_t xconftime;			/* Modification time of xconf */
  struct usermail			/* User's private mail room */
  {
    long    num;			/* Universal message number          */
    long    pos;			/* Message positions in master file  */
  } mr[MAILMSGS];
  char    aideinfo[81];			/* Just for the Sproinglet */
  char    reminder[81];			/* remind yourself of whatever */
  char    loginname[16];		/* New larger field for loginname */
  char    unused0;			/* (WAS) Anonymous aide/wizard */
  long    totalx;			/* Number of X's sent */
  char    A_real_name[MAXNAME + 1];	/* User's real name */
  char    A_addr1[MAXNAME + 1];		/* Home address */
  char    A_city[21];			/* User's city */
  char    A_state[21];			/* Users's state or country */
  char    A_zip[11];			/* Zipcode or mailcode */
  char    A_phone[21];			/* Phone in format (AAA)-NNN-NNNN */
  char    A_mail[41];			/* User's internet mail address */
  char    doing[40];			/* What the hell are you doing? */
  char    vanityflag[40];		/* Have a nice vanity flag */
  short   btmpindex;			/* Index to user's record in btmp */
  long    xseenpos;
  long    xmaxpos;
  short   xmsgnum;
  long    xminpos;
/* remember alignment issues when adding more */
  unsigned short yells;			/* Yells answered as sysop */
  unsigned short vals;			/* Validations done as sysop */
};


struct bigbtmp
{
  long users;			/* Users online now */
  long queued;			/* Users queued now */
  short index[MAXUSERS];	/* Indices into btmp struct array */
  time_t ghostcheck;		/* Cheap hack */
  long eternal;			/* Eternal number (another check hack) */

  short qp;			/* Index to end of queue */
  short oldqp;
  short maxqp;			/* Maximum users in queue so far */
  short socks;			/* Total sockets connected at this time */
  long forks;			/* Total number of forks so far */
  long reaps;			/* Total number of reaps so far */
  short connectable;		/* Allowable connects during next 30 seconds */
  short startable;		/* Allowable forks during next 30 seconds */
  long aidewiz;			/* Number of aide/wizard logins */
  long upgrade;			/* Number of upgraded user logins */
  long nonupgrade;		/* Number of regular user logins */
  short starts;			/* Forks in last 30 second period */
  short qflag;			/* Set when connection dropped */
  char hello[1000];		/* Hello message */
  char unused[16];		/* Space made as unused */
  char sig_alrm;
  char sig_cld;
  char sig_hup;
  char sig_term;
  char sig_quit;
  char sig_urg;
  char sig_usr1;
  char sig_usr2;
  short hellolen;		/* Length of hello message */
  char down;			/* True if BBS down */
  short limit;			/* User limit */
  long cpuuser;			/* Save old value of user CPU */
  long cpusys;			/* Save old value of sys CPU */
  pid_t pid;			/* Pid of running queue program */
  fd_set fds;
  time_t t;
  struct tm *ltm;
  time_t lastcheck;
  long oldforks;
  char lockout;
  char init_info;
  char init_reread;
  short qindex[MAXQ];		/* Queue of fd's */

  struct btmp
  {
    long eternal;
    long usernum;
    time_t time;
    uint32_t remaddr;
    uint16_t remport;
    pid_t pid;
    char name[MAXALIAS + 1];
    char remote[40];
    char doing[40];
    char remlogin[16];
    char xstat;
    char elf;
    char _was_anonymous;
    char guest;
    char connecting;
    char client;
    char nox;
    char unused;
    short sleeptimes;
    long ulink;
  } btmp[MAXUSERS];

  struct qtmp
  {
    struct in_addr addr;/* Peer addresses of fd's */
    time_t conn;		/* Time connected */
    time_t last;		/* Time last data arrived */
    short qlo;			/* Lowest point of fd in queue so far */
    char acc;			/* Count of useless keypresses */
    char sgaloop;		/* SGA loop counter */
    char echoloop;		/* ECHO loop counter */
    char client;		/* Is connected as a client */
    char new;			/* Flag set to create new user */
    char checkup;		/* Needs to check up on position */
    char wasinq;		/* Was in queue at some point */
    char login;			/* State counter for login */
    char name[20];		/* BBS username */
    char pass[9];		/* BBS passwd */
    char options[64];
    char do_dont_resp[64];
    char will_wont_resp[64];
    char remoteusername[16];
    char rows;
    char initstate;
    char state;
    char ncc;
    unsigned char netibuf[32];
    unsigned char *netip;
    unsigned char netobuf[16];
    unsigned char *nfrontp;
    unsigned char *nbackp;
    unsigned char subbuffer[64];
    unsigned char *subpointer;
    unsigned char *subend;
    uint16_t port;
    short wouldbe;
    char unused[4];
  } qt[MAXQ];
  /* unused? */
};


#define SEM_MSG		0
#define SEM_XMSG	1
#define SEM_USER	2
#define SEM_BTMP	3
#define SEM_NEWBIE	4
#define SEM_INDEX	5
#define SEM_VOTE	6


struct msg
{
  int sem[7];
  long    eternal;			/* New user number */
  long    highest;			/* highest message number in file   */
  long    curpos;			/* notate where the next msg will go */
  long    xcurpos;
  long    bcastpos;
  time_t  lastbcast;
  short   maxusers;			/* Max users ever */
  short   maxqueue;			/* Max queue ever */
  short   maxtotal;			/* Max total ever */
  short   unused1;
  long    xmsgsize;			/* Size of X message data file */
  short   maxnewbie;			/* Index of max newbie */
  short   unused2;
  time_t  t;				/* Close to current time */
  char    unused3[24];
  struct room
  {
    char    name[MAXNAME + 1];		/* Max. len is 39, plus null term   */
    long    roomaide;			/* User number of room aide         */
    long    highest;			/* Highest message NUMBER in room   */
    long    posted;			/* How many msgs posted in room     */
    char    passwd[11];			/* passworded room */
    long    num[MSGSPERRM];		/* Universal message number   */
    long    chron[MSGSPERRM];		/* chronological order in room of note*/
    long    pos[MSGSPERRM];		/* Message positions in master file */
    long    descupdate;			/* Eternal of last room desc update */
    unsigned char flags;                /* See flag values above            */
    char    gen;			/* Generation number of room        */
    char    unused[6];
  } room[MAXROOMS];
  struct newbie
  {
    char   name[20];
    time_t time;
  } newbies[MAXNEWBIES];
  char shit[108];
};

#define M_MAGIC			0xfd

struct mheader
{
  unsigned int magic:8;
  unsigned int poster:24;
  unsigned int :5;
  unsigned int quotedx:1;
  unsigned int mail:1;
  unsigned int approval:1;
  unsigned int hlen:6;
  unsigned int len:18;
  unsigned long msgid;
  unsigned short forum;
  unsigned char mtype;
  time_t ptime;

  union
  {
    struct
    {
      unsigned int :8;
      unsigned int recipient:24;
    } mail;
  } ext;
};


#define X_NORMAL	0
#define X_QUESTION	1
#define X_BROADCAST	2

struct xheader
{
  unsigned int checkbit:1;
  unsigned int :6;
  unsigned int type:3;
  unsigned long snum;
  unsigned long rnum;
  long sprev;
  long snext;
  long rprev;
  long rnext;
  time_t time;
};

#define XPENDING	(msg->lastbcast > lastbcast || (ouruser && ouruser->xseenpos && (!mybtmp->nox || ouruser->f_xmsg)))
