#include <string.h>
/*
 * defs.h - All needed include files.
 */
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#ifdef _WHIP
#include "whip.h"
#else
#include <stdio.h>
#include <ctype.h>
#endif
#include <sys/types.h>
#include <sys/param.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <setjmp.h>
#include <syslog.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <sys/resource.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#ifdef __hpux
#include <sys/pstat.h>
#include <sys/rtprio.h>
#endif

#ifdef _SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif


#ifndef FALSE
#define FALSE		0
#define TRUE		1
#endif

#define NO		0
#define YES		1
#define OFF		0
#define ON		1

#define STDINBUFSIZ	256
#define STDOUTBUFSIZ	960

#ifndef MIN
#define MIN(a, b)       ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b)	((a) > (b) ? (a) : (b))
#endif

#ifndef MAP_FILE
#define MAP_FILE 0
#endif

/* define this to allow the bbs to logout users after n minutes */
#define IDLELOGOUT	1440

/* define this if you want users logged out after 6000 minutes.  I should
   make this smarter */
#define	TOTALTIMELIMIT

#include "bbs.h"
#include "proto.h"
#ifdef INQUEUE
#include "qtelnet.h"
#else
#include "telnet.h"
#endif
#include "users.h"
#include "queue.h"

extern char **environ;

#define BBS             1
#define FINGER          2
#define QUEUE           3
#define INIT            4
#define SYNC            5
#define UPDATE          6
#define BACKUP          7

#ifdef _WHIP
#undef SOMAXCONN
#define SOMAXCONN 128
#endif

#if 1
#define FLUSH(a,b)	((b = q->qt[a].nfrontp - q->qt[a].nbackp) ? (!ssend(a, q->qt[a].nbackp, b) ? ((q->qt[a].nbackp = q->qt[a].nfrontp = q->qt[a].netobuf), 0) : -1) : 0)
#else
#define FLUSH(a,b)	if (b = q->qt[a].nfrontp - q->qt[a].nbackp)\
			{\
				if (!ssend(a, q->qt[a].nbackp, b))\
					q->qt[a].nbackp = q->qt[a].nfrontp = q->qt[a].netobuf;\
			}
#endif


/*
 * Sure would be nice if there was a standard interface to this kind of
 * information, it is very useful.
 */
#ifdef linux
#ifdef _IO_file_flags
#define INPUT_LEFT()   ((stdin)->_IO_read_ptr < (stdin)->_IO_read_end)
#else
#define INPUT_LEFT()   ((stdin)->_gptr < (stdin)->_egptr)
#endif
#else
#if (BSD >= 44) || defined(bsdi)
#define INPUT_LEFT()  ((stdin)->_r > 0)
#else
#define INPUT_LEFT()   ((stdin)->_cnt > 0)
#endif
#endif

/* FreeBSD 4.4 spelling errors */
#ifndef MAP_ANONYMOUS
# define MAP_ANONYMOUS MAP_ANON
#endif
#ifndef O_SYNC
# define O_SYNC O_FSYNC
#endif
#ifndef SIGCLD
# define SIGCLD SIGCHLD
#endif


