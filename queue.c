#define INQUEUE
#include "defs.h"
#include "ext.h"

void
bbsqueue(int dofork)
{
register int x;
register int s;
int i;

  //sigblock(sigmask(SIGCLD) | sigmask(SIGALRM) | sigmask(SIGHUP) | sigmask(SIGUSR2) | sigmask(SIGTERM) | sigmask(SIGQUIT));
  openlog("bbsqueued", LOG_PID, LOG_LOCAL0);
  q = bigbtmp;

  chdir(ROOT"/core/bbsqueued");
  umask(027);
  //setresgid(19, 19, 19);
  //setresuid(0, 0, 0);

  if (q->pid != getpid() || !getenv("BBSQUEUED"))
  {
    bzero(q->qt, sizeof q->qt);
    q->qp = 0;
    q->oldqp = 0;
    q->maxqp = 0;
    q->socks = 0;
    q->forks = 0;
    q->reaps = 0;
    q->connectable = 0;
    q->startable = 0;
    q->aidewiz = 0;
    q->nonupgrade = 0;
    q->starts = 0;
    q->qflag = 0;
    q->cpuuser = 0;
    q->cpusys = 0;
    q->init_reread = 0;
    if (dofork && fork())
      _exit(0);
    q->pid = getpid();

/*
    for (i = 5 - 1; i >= 0; i--)
      close(i);
*/ 
    setsid();
    {
      struct rlimit rlp;

      getrlimit(RLIMIT_NOFILE, &rlp);
      rlp.rlim_cur = MAXQ;
      setrlimit(RLIMIT_NOFILE, &rlp);
    }

    /* Start up */
    signal(SIGHUP, (void *)reread);
    signal(SIGQUIT, (void *)setup);
    do_setup();
    syslog(LOG_INFO, "Queue started");
    do_reread();
    do_ring();
  }
  else
  {
    close(1);
    close(2);
    syslog(LOG_INFO, "Restarted");
    if (q->limit > 0)
      signal(SIGCLD, (void *)reap);
    else
      signal(SIGCLD, SIG_IGN);
    signal(SIGHUP, (void *)reread);
    signal(SIGQUIT, (void *)setup);
  }

  nice(-20);
  nice(-20);
  nice(20);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGTERM, (void *)quit);
  signal(SIGUSR2, (void *)restart);
  signal(SIGALRM, (void *)ring);
  alarm(30);
  //sigsetmask(0);
  signal(SIGUSR1, (void *)dump);

  /* Placeholder for stdout (syslog fd) and stderr (copy of syslog fd) */
  dup2(1, 2);
  x = 0;
  if (!q->socks)
    bigbtmp->queued = 0;


  for (;;)
  {
    if (q->connectable)
      FD_SET(sfd, &q->fds);
    else
      FD_CLR(sfd, &q->fds);

    for (i = 0, x = 4; x < MAXQ; x++)
      if (q->qt[x].conn && !q->qt[x].ncc)
      {
        FD_SET(x, &q->fds);
	i = x;
      }
      else
        FD_CLR(x, &q->fds);

    if (select(MAX (sfd, i) + 1, &q->fds, 0, 0, 0) < 0)
      i = errno;
    else
      i = 0;
    q->t = time(0);
    /* q->ltm = localtime(&q->t); */	/* segfault here... ? */
    if (i)
      if (i == EINTR)
      {
	if (f_term)
	  do_quit();
	if (f_qalarm)
	  do_ring();
	if (f_reread)
	  do_reread();
	if (f_quit)
	  do_setup();
	if (f_restart)
	  do_restart();
        runbbs(0);
        continue;
      }
      else
        logfatal("select: %m");

    while (FD_ISSET(sfd, &q->fds))
    {
      struct sockaddr_in sa;

      q->connectable--;
      q->qt[0].last = q->t;
      i = sizeof sa;
      while ((x = accept(sfd, (struct sockaddr*)&sa, &i)) < 0)
      {
        if (errno == EINTR)
	  continue;
	if (errno == EMFILE)
	{
	  syslog(LOG_WARNING, "accept: %m");
	  q->connectable = 0;
	  break;
	}
        logfatal("accept: %m");
      }

      {
        struct linger linger;

        linger.l_onoff = 1;
        linger.l_linger = 0;
        if (setsockopt(x, SOL_SOCKET, SO_LINGER, &linger, sizeof linger) < 0)
        {
          syslog(LOG_WARNING, "setsockopt on fd %d SO_LINGER: %m", x);
          if (close(x) < 0)
            syslog(LOG_WARNING, "SO_LINGER failure: close: %m");
          break;
        }
      }
      i = 1;
      if (setsockopt(x, SOL_SOCKET, SO_OOBINLINE, &i, sizeof i) < 0)
      {
        syslog(LOG_WARNING, "setsockopt on fd %d SO_OOBINLINE: %m", x);
        if (close(x) < 0)
          syslog(LOG_WARNING, "SO_OOBINLINE failure: close: %m");
        break;
      }
      if (setsockopt(x, SOL_SOCKET, SO_KEEPALIVE, &i, sizeof i) < 0)
      {
        syslog(LOG_WARNING, "setsockopt on fd %d SO_KEEPALIVE: %m", x);
        if (close(x) < 0)
          syslog(LOG_WARNING, "SO_KEEPALIVE failure: close: %m");
        break;
      }
      if (fcntl(x, F_SETFL, O_NONBLOCK) < 0)
      {
        syslog(LOG_WARNING, "fcntl on fd %d: %m", x);
        if (close(x) < 0)
          syslog(LOG_WARNING, "fcntl failure: close: %m");
        break;
      }

      q->socks++;
      if (q->lockout && (sa.sin_addr.s_addr >= 0x80ff3865UL && sa.sin_addr.s_addr <= 0x80ff3868UL))
      {
	if (((!q->ltm->tm_wday || q->ltm->tm_wday == 6) && q->ltm->tm_hour >= 12) || q->ltm->tm_hour >= 14 || (q->ltm->tm_wday && q->ltm->tm_wday < 6 && !q->ltm->tm_hour))
        {
          if (!ssend(x, PORTALLOCKOUT, sizeof PORTALLOCKOUT - 1))
            drop(-x);
          break;
        }
      }

/*
      if (ssend(x, q->hello, q->hellolen - 1))
        break;
*/

      if (q->down && sa.sin_addr.s_addr != 0x7f000001)
      {
        drop(-x);
        break;
      }

      q->qt[x].conn = q->qt[x].last = q->t;
      q->qt[x].netip = q->qt[x].netibuf;
      q->qt[x].nfrontp = q->qt[x].nbackp = q->qt[x].netobuf;
      q->qt[x].subpointer = q->qt[x].subend = q->qt[x].subbuffer;
      *q->qt[x].remoteusername = 0;
      q->qt[x].rows = 24;
      q->qt[x].initstate = T_INIT1;
      q->qt[x].state = TS_DATA;
      for (i = 0; i < sizeof q->qt[0].options; i++)
        q->qt[x].options[i] = q->qt[x].do_dont_resp[i] = q->qt[x].will_wont_resp[i] = 0;
      q->qt[x].ncc = 0;
      q->qt[x].addr = sa.sin_addr.s_addr;
      q->qt[x].port = sa.sin_port;
      q->qt[x].acc = 0;
      q->qt[x].login = 0;
      q->qt[x].sgaloop = 0;
      q->qt[x].echoloop = 0;
      q->qt[x].client = 0;
      q->qt[x].checkup = 0;
      q->qt[x].new = 0;
      q->qt[x].wasinq = 0;
      break;
    }

    for (x = 4; x < MAXQ; x++)
    {
      if (!q->qt[x].conn)
        continue;

      if (FD_ISSET(x, &q->fds))
      {
        while ((q->qt[x].ncc = recv(x, q->qt[x].netibuf, sizeof q->qt[x].netibuf, 0)) < 0 && errno == EINTR)
          ;
        if (q->qt[x].ncc <= 0)
	{
          drop(x);
	  continue;
	}
        q->qt[x].netip = q->qt[x].netibuf;
        q->qt[x].last = q->t;
      }

      if (q->qt[x].ncc > 0)
      {
        qtelrcv(x);
        if (!q->qt[x].conn)
	  continue;
      }

      if (q->qt[x].checkup)
      {
        q->qt[x].checkup = 0;
        runbbs(-x);
      }

      if (q->qt[x].initstate)
      {
        if (FLUSH(x, i) < 0)
	  continue;
        qinit(x);
        if (!q->qt[x].conn)
	  continue;
      }

      if (FLUSH(x, i) < 0)
	continue;
    }
    runbbs(0);
  }
}
