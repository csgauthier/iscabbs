#include "defs.h"
#include "ext.h"


void
reap(void)
{
  int stat_loc;

  while (wait(&stat_loc) < 0 && errno == EINTR)
    ;
  if (!WIFSTOPPED(stat_loc))
    q->reaps++;
  signal(SIGCLD, (void *)reap);
}



void
ring(void)
{
  if (++f_qalarm > 4)
  {
    if (fork())
    {
      close(0);
      syslog(LOG_INFO, "Queue looping, aborting and performing restart...");
      dump();
    }
    else
    {
      execl(BBSEXEC, BBSEXEC, "-q", NULL);
    }
  }
  signal(SIGALRM, (void *)ring);
  alarm(30);
}



void
dump(void)
{
  struct bigbtmp *temp;

  temp = (struct bigbtmp *)alloca(sizeof(struct bigbtmp));
  bcopy(q, temp, sizeof(struct bigbtmp));
  q = bigbtmp = temp;
  raise(SIGABRT);
}



void
do_ring(void)
{
char pst_data[128];
struct pst_dynamic *pst = (struct pst_dynamic *)(void *)pst_data;
int z;

  f_qalarm = 0;
  q->t = time(0);
  for (z = 3; z < MAXQ; z++)
    if (q->qt[z].conn || q->qt[z].state < 0)
      if ((q->qt[z].initstate && q->qt[z].last + 300 < q->t)
          || (q->qt[z].state < 0 && q->qt[z].last + 9 < q->t))
        drop(z);
  if (q->qt[0].last + 300 < q->t)
    setup();
/*
  if (q->limit < 0)
  {
    pstat_getdynamic((void *)pst, sizeof pst_data, 0, 0);
    if (!q->cpuuser)
      q->startable = -q->limit - (int)pst->psd_avg_1_min * 2 - (int)pst->psd_avg_15_min * 2;
    else
    {
      q->startable = (-q->limit - ((int)pst->psd_avg_1_min * 2 + (pst->psd_cpu_time[0] + pst->psd_cpu_time[2] - q->cpuuser - q->cpusys) / ((q->lastcheck && (q->t - q->lastcheck)) ? (q->t - q->lastcheck) : 1))) / 2;
      if (q->qp > -q->limit / 8 && q->startable > -q->limit / 8 && q->startable > -q->limit - (int)pst->psd_avg_15_min * 4)
        q->startable = -q->limit - (int)pst->psd_avg_15_min * 4;
#if 1
  Temporary hack while whip == liquefy
      if (bigbtmp->users > 4100)
        q->startable = MIN(q->startable, q->qp / 2);
      if (bigbtmp->users > 4200)
	q->startable = MIN(q->startable, q->qp / 16);
#endif
      q->oldforks = q->forks;
    } 
    q->lastcheck = q->t;
    q->cpuuser = pst->psd_cpu_time[0];
    q->cpusys = pst->psd_cpu_time[2];
    if (q->users < MAXUSERS / 2)
      q->connectable = MAXUSERS / 8;
    else
      q->connectable = MAXACTIVITY;
  }
  else */
  {
    q->startable = MAXACTIVITY;
    q->connectable = MAXACTIVITY;
  }
}



void
reread(void)
{
  f_reread = 1;
  signal(SIGHUP, (void *)reread);
}



void
do_reread(void)
{
char buf[256];
FILE *f;
int c;
int limit;
int lockout;
int i;

  f_reread = 0;
  if (!(f = fopen(DOWNFILE, "r")))
    if (!(f = fopen(HELLOFILE, "r")))
      logfatal("HELLOFILE fopen: %m");
    else
      q->down = 0;
  else
    q->down = 1;
  setvbuf(f, buf, _IOFBF, sizeof buf);
  q->hellolen = 0;
  while (q->hellolen + 1 < sizeof q->hello)
  {
    if ((c = getc(f)) == '\n')
      q->hello[q->hellolen++] = '\r';
    else if (c < 0)
      break;
    q->hello[q->hellolen++] = c;
  }
  fclose(f);

  if ((i = open(LIMITFILE, O_RDONLY)) < 0)
    logfatal("LIMITFILE open: %m");
  read(i, buf, sizeof buf);
  close(i);
#if 1
  if (sscanf(buf, "%d %d", &limit, &lockout) != 2)
    logfatal("scanf: bad format!");
#else
#ifdef TEST
  syslog(LOG_INFO, "Lockout and limit frozen at 0 & on!");
  limit = 0;
  lockout = 1;
#else
  syslog(LOG_INFO, "Lockout and limit frozen at -150 & on!");
  limit = -150;
  lockout = 1;
#endif
#endif
  if (q->init_reread && q->lockout != lockout)
    syslog(LOG_INFO, "Lockout turned %s", lockout ? "on" : "off");
  q->lockout = lockout;
  if (q->init_reread && q->limit != limit)
    syslog(LOG_INFO, "New limit %d", limit);
  q->init_reread = 1;
  q->limit = limit;
  if (limit > 0)
    signal(SIGCLD, (void *)reap);
  else
    signal(SIGCLD, SIG_IGN);
}



void
quit(void)
{
  f_term = 1;
  signal(SIGTERM, (void *)quit);
}



void
do_quit(void)
{
  syslog(LOG_INFO, "users %ld, queue %d, limit %d, lockout %d", q->forks - q->reaps, q->qp, q->limit, q->lockout);
  syslog(LOG_INFO, "forks %ld, maxqueue %d", q->forks, q->maxqp);
  syslog(LOG_INFO, "admins %ld, upgrades %ld, users %ld", q->aidewiz, q->upgrade, q->nonupgrade);
  _exit(0);
}



void
restart(void)
{
  f_restart = 1;
  signal(SIGUSR2, (void *)restart);
}



void
do_restart(void)
{
  char *newenv[2];

  newenv[0] = "BBSQUEUED=1";
  newenv[1] = 0;
  environ = newenv;
  syslog(LOG_INFO, "Restarting process");
  execl(BBSEXEC, BBSEXEC, "-q", NULL);
  syslog(LOG_ERR, "exec: %m");
}



void
setup(void)
{
  f_quit = 1;
  signal(SIGQUIT, (void *)setup);
}



void
do_setup(void)
{
struct sockaddr_in sa;
int on = 1;
long oldmask;

  f_quit = 0;
/*
  if (!close(0))
    syslog(LOG_INFO, "Rebound listener socket");
*/
  if (sfd)
    close (sfd);

  if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    logfatal("socket: %m");
  if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on))
    logfatal("setsockopt: %m");
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = 0;
  sa.sin_port = htons (PORT);
  if (bind(sfd, (const struct sockaddr*)&sa, sizeof sa))
    logfatal("bind: %m");
  if (listen(sfd, SOMAXCONN))
    logfatal("listen: %m");
  q->qt[0].last = time(0);
}



void
checkauth(int x)
{
char work[80];
struct user *u;
char *p;
int i, j;

  if (!(u = getuser(q->qt[x].name)))
  {
    ssend(x, INCORRECT, sizeof INCORRECT - 1);
    q->qt[x].login = 0;
    return;
  }

  p = (char *)crypt(q->qt[x].pass, u->passwd);
  /* Clear password for security -- it is cleartext in the file! */
  strncpy(q->qt[x].pass, "", sizeof q->qt[x].pass);
  if (strncmp(u->passwd, p, 13))
  {
    ssend(x, INCORRECT, sizeof INCORRECT - 1);
    q->qt[x].login = 0;
    freeuser(u);
    return;
  }

  for (i = MAILMSGS - 1, j = 0; i >= 0 && u->lastseen[MAIL_RM_NBR] < u->mr[i].num; i--)
    if (u->mr[i].pos > 0)
    { 
      if (j++ < 0)
        j++;
    }
    else if (!j)
      j--;

  if (u->f_admin)
  {
    q->aidewiz++;
    ssend(x, AIDELOGGEDIN, sizeof AIDELOGGEDIN - 1);
    q->qt[x].login = -4;
    if (q->qt[x].new)
    {
      syslog(LOG_INFO, "Sysop NEW login by %s", q->qt[x].name);
      strcpy(q->qt[x].name, "New");
    }
  }
  else
  {
    q->nonupgrade++;
    ssend(x, LOGGEDIN, sizeof LOGGEDIN - 1);
    q->qt[x].login = -1;
  }
  q->qt[x].checkup = 1;

  if (j == 1)
    ssend(x, HASONEMAIL, sizeof HASONEMAIL - 1);
  else if (j > 1)
  {
    sprintf(work, HASMANYMAIL, j);
    ssend(x, work, strlen(work));
  }

  freeuser(u);
}


void
dologin(int c, int x)
{
  char d;

  if (q->qt[x].login == 5)
  {
    if (ssend(x, "\r\nName: ", 8))
      return;
    q->qt[x].login = 10;
  }
  else if (q->qt[x].login >= 10 && q->qt[x].login < 30)
  {
    if (c == '_')
      c = ' ';
    if (c == '\r' || c == '\n')
      if (q->qt[x].login == 10)
      {
        q->qt[x].login = 0;
        ssend(x, "\r\n", 2);
      }
      else
      {
        q->qt[x].name[q->qt[x].login - 10] = '\0';
	if (!strcmp(q->qt[x].name, "New"))
	{
	  ssend(x, NEWUSERCREATE, sizeof NEWUSERCREATE - 1);
	  q->qt[x].new = 1;
	  q->qt[x].login = 10;
	  return;
	}
        q->qt[x].login = 30;
        ssend(x, "\r\nPassword: ", 12);
      }
    else if ((c == 8 || c == 127) && q->qt[x].login > 10)
    {
      ssend(x, ERASE, 3);
      q->qt[x].login--;
    }
    else if ((c == 24 || c == 21) && q->qt[x].login > 10)
    {
      ssend(x, ERASE, (q->qt[x].login - 10) * 3);
      q->qt[x].login = 10;
    }
    else if (((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == ' ') && q->qt[x].login < 29)
    {
      if (c >= 'a' && c <= 'z' && (q->qt[x].login == 10 || q->qt[x].name[q->qt[x].login - 11] == ' '))
        c -= 32;
      d = c;
      ssend(x, &d, 1);
      q->qt[x].name[q->qt[x].login++ - 10] = c;
    }
    return;
  }
  else if (q->qt[x].login >= 30)
  {
    if (c == '\r' || c == '\n')
    {
      q->qt[x].pass[q->qt[x].login - 30] = '\0';
      checkauth(x);
    }
    else if ((c == 8 || c == 127) && q->qt[x].login > 30)
    {
      ssend(x, ERASE, 3);
      q->qt[x].login--;
    }
    else if (c == 24 && q->qt[x].login > 30)
    {
      ssend(x, ERASE, (q->qt[x].login - 30) * 3);
      q->qt[x].login = 30;
    }
    else if (c >= ' ' && q->qt[x].login < 38)
    {
      q->qt[x].pass[q->qt[x].login++ - 30] = c;
      ssend(x, ".", 1);
    }
  }
}



void
logfatal(char *error)
{
  struct timeval tv;

  syslog(LOG_ERR, "%s", error);
#if 1
  syslog(LOG_INFO, "Starting fresh queue process upon death in 15 seconds...");
  sleep(15);
  execl(BBSEXEC, BBSEXEC, "-q", NULL);
#endif
  _exit(1);
}

void
drop(int s)
{
int i, j;

  if (s > 0)
  {
    close(s);
    q->socks--;
  }
  else
  {
    s = (-s);
    shutdown(s, 2);
    q->qt[s].state = -1;
  }
  q->qt[s].conn = 0;
  FD_CLR(s, &q->fds);
  for (i = 0; i < q->qp; i++)
    if (q->qindex[i] == s)
    {
      for (q->qflag++, q->qp--, j = i; j < q->qp; j++)
        q->qindex[j] = q->qindex[j + 1];
      break;
    }
}


int
ssend(int s, char* msg, int len)
{
  int x;

  for (;;)
  {
    x = send(s, msg, len, 0);
    if (x == len)
      return(0);
    if (x < 0)
    {
      if (errno == EINTR)
        continue;
      else
      {
        drop(s);
        return(-1);
      }
    }
    msg += x;
    len -= x;
    continue;
  }
}
