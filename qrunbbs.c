#include "defs.h"
#include "ext.h"


void
runbbs(s)
int s;
{
int len;
int i, j;
short sqp;
int pick;
char str[160];
char date[12];
struct hostent *hp;
char *host;
char bbsname[28];
char lines[9];
int y;
u_long addr;
u_short port;
int client;
char remoteusername[16];
char *newenv[3];
struct sockaddr_in sa;
struct linger linger;
struct timeval tv;
char identname[16];
int fds;
char *p;

  if (s > 0)
  {
    if (!q->qt[s].conn)
      return;
    if (LOCAL(q->qt[s].addr))
    {
      for (i = 0; q->qp > i && LOCAL(q->qt[q->qindex[i]].addr); i++)
        ;
      for (j = q->qp; j > i; j--)
        q->qindex[j] = q->qindex[j - 1];
      q->qindex[i] = s;
      q->qt[s].qlo = ++(q->qp);
    }
    else if (q->qt[s].client)
    {
      for (i = 0; q->qp > i && q->qt[q->qindex[i]].client; i++)
        ;
      for (j = q->qp; j > i; j--)
        q->qindex[j] = q->qindex[j - 1];
      q->qindex[i] = s;
      q->qt[s].qlo = ++(q->qp);
    }
    else
    {
      q->qindex[q->qp++] = s;
      q->qt[s].qlo = q->qp;
      for (i = 0; q->qp > i && q->qt[q->qindex[i]].client; i++)
        ;
      q->qt[s].wouldbe = i;
    }
#ifndef TEST
    if (q->qp > q->maxqp)
      q->maxqp = q->qp;
    if (q->qp > msg->maxqueue)
      msg->maxqueue = q->qp;
    i = bigbtmp->users + q->qp;
    if (i > msg->maxtotal)
      msg->maxtotal = i;
#endif
  }
  else if (s < 0 && (q->qt[-s].login == -4 || q->qt[-s].login == -3))
  {
    for (i = 0; q->qt[q->qindex[i]].login <= q->qt[-s].login - 10; i++)
      ;
    for (pick = (-s), j = q->qp - 1; j >= i; j--)
      if (!pick)
        q->qindex[j + 1] = q->qindex[j];
      else if (q->qindex[j] == pick)
        pick = 0;
    q->qindex[i] = -s;
    q->qt[-s].login -= 10;
  }

  while (q->qp)
  {
    if ((q->limit >= 0 && q->limit <= q->forks - q->reaps) || q->startable <= 0)
    {
      if (q->qflag || s)
        i = q->qp - 1;
      else
        break;
      if (s > 0 && !LOCAL(q->qt[s].addr) && !q->qt[s].wasinq)
      {
        ssend(s, BBSFULL, sizeof BBSFULL - 1);
	q->qt[s].wasinq = 1;
        if (!q->qt[s].client)
        {
          len = sprintf(str, IFYOUHADCLIENT, q->qt[s].wouldbe);
          ssend(s, str, len);
        }
      }
      sprintf(date, "%02d:%02d:%02d", q->ltm->tm_hour, q->ltm->tm_min, q->ltm->tm_sec);
      for (; i >= 0; i--)
      {
        if (!s && q->qt[q->qindex[i]].qlo <= i || s < 0 && q->qindex[i] != -s || s <= 0 && q->qt[q->qindex[i]].login >= 10 || s > 0 && q->qindex[i] != s || LOCAL(q->qt[q->qindex[i]].addr) || q->qt[q->qindex[i]].login == -14)
          continue;
        if (!i || !q->qt[q->qindex[i]].qlo)
          len = sprintf(str, ATFRONT, date, (q->t - q->qt[q->qindex[i]].conn) / 60, (q->t - q->qt[q->qindex[i]].conn) % 60);
        else if (i == 1 || q->qt[q->qindex[i]].qlo == 1)
          len = sprintf(str, ONEAHEAD, date, (q->t - q->qt[q->qindex[i]].conn) / 60, (q->t - q->qt[q->qindex[i]].conn) % 60, bigbtmp->users, q->qp);
        else
          len = sprintf(str, MANYAHEAD, date, (q->t - q->qt[q->qindex[i]].conn) / 60, (q->t - q->qt[q->qindex[i]].conn) % 60, i <= q->qt[q->qindex[i]].qlo ? i : q->qt[q->qindex[i]].qlo, bigbtmp->users, q->qp);
        if (i < q->qt[q->qindex[i]].qlo)
          q->qt[q->qindex[i]].qlo = i;
        ssend(q->qindex[i], str, len);
      }
      if (q->qt[q->qindex[0]].login != -14 && !LOCAL(q->qt[q->qindex[0]].addr))
        break;
    }
    else
      if (s < 0 && (q->qt[q->qindex[0]].login != -14 && !LOCAL(q->qt[q->qindex[0]].addr)))
        break;

    for (pick = 0; pick < q->qp; pick++)
    {
      if (q->qt[i = q->qindex[pick]].conn && (q->qt[i].state == TS_DATA || q->qt[i].state == TS_CR) && !q->qt[i].ncc)
      {
        if (pick >= q->qp || q->qt[i].login != -14 && !LOCAL(q->qt[i].addr) && ((q->limit >= 0 && q->limit <= q->forks - q->reaps) || q->startable <= 0))
          continue;
        FLUSH(i, j);
        if (q->qt[i].conn && (!q->qt[i].wasinq || !ssend(i, "\007\n", 2)))
          break;
        pick--;
      }
#if 0
      else if (!pick && (q->qt[i].login == -14 || LOCAL(q->qt[i].addr)))
      {
        pick = -1;
        break;
      }
#else
      else
	if (q->qt[i].last + 300 < q->t)
	  syslog(LOG_INFO, "Time difference %d for %08x", q->t - q->qt[i].last, q->qt[i].addr);
#endif
    }

    if (pick >= q->qp || pick < 0)
      break;

    y = q->qindex[pick];
    addr = q->qt[y].addr;
    port = q->qt[y].port;
    client = q->qt[y].client;
    strcpy(remoteusername, q->qt[y].remoteusername);
    sprintf(newenv[0] = lines, "ROWS=%d", q->qt[y].rows);
    if (q->qt[y].login < 0)
      sprintf(newenv[1] = bbsname, "BBSNAME=%s", q->qt[y].name);
    else
      newenv[1] = 0;
    newenv[2] = 0;
    environ = newenv;

    if (i = fork())
      if (i < 0)
        if (errno == EAGAIN)
        {
          syslog(LOG_WARNING, "Out of processes, sleeping for 10 seconds...");
	  sleep(10);
          continue;
        }
        else
          logfatal("fork: %m");
      else
      {
        drop(y);
        q->forks++;
        q->startable--;
        q->qflag++;
      }
    else
    {
      alarm(0);
      signal(SIGALRM, SIG_IGN);
      signal(SIGHUP, SIG_IGN);
      signal(SIGUSR2, SIG_DFL);
      signal(SIGTERM, SIG_IGN);
      signal(SIGCLD, SIG_DFL);
      signal(SIGQUIT, SIG_DFL);
      if (fcntl(y, F_SETFL, 0) < 0)
        logfatal("fcntl: %m");
      closelog();
      /* used to be getnumfds() */
      for (i = 25 - 1; i >= 0; i--)
        if (i != y)
          close(i);
      dup2(y, 0);
      if (y >= 1)
        close(y);

      bzero((char *)&sa, sizeof sa);
      if (!*remoteusername && (s = socket(AF_INET, SOCK_STREAM, 0)) > 0)
      {
	linger.l_onoff = 1;
        linger.l_linger = 0;
        setsockopt(s, SOL_SOCKET, SO_LINGER, &linger, sizeof linger);
	fcntl(s, F_SETFL, O_NONBLOCK);
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = addr;
        sa.sin_port = htons(113);
	connect(s, (const struct sockaddr*)&sa, sizeof sa);
	tv.tv_sec = 4;
	tv.tv_usec = 0;
	fds = 1 << s;
	if ((y = select(s + 1, (fd_set *)&fds, (fd_set *)&fds, 0, &tv)) > 0)
        {
	  sprintf(str, "%d,%d\r\n", ntohs (port), PORT);
          write(s, str, strlen(str));
	  shutdown(s, 1);
	}
	else
	  close(s);
      }
      else
        y = -1;


      hp = gethostbyaddr ((char*) &addr, 4, AF_INET);
      if (hp)
        host = hp->h_name;
      else
        host = inet_ntoa(*(struct in_addr *)&addr);

      if (y > 0)
      {
	tv.tv_sec = 2;
	tv.tv_usec = 0;
	fds = 1 << s;
	if (select(s + 1, (fd_set *)&fds, 0, 0, &tv) > 0)
	{
	  bzero((void *)str, 80);
	  read(s, str, 79);
	  if (p = (char *)strstr(str, "USERID"))
	  {
	    for (p += 6; *p && *p++ != ':'; )
	      ;
	    for (; *p && *p++ != ':'; )
	      ;
	    for (; *p && isspace(*p); p++)
	      ;
	    strncpy(identname, p, sizeof identname - 1);
	    identname[sizeof identname - 1] = 0;
	    for (p = identname + sizeof identname - 2; !*p || isspace(*p); p--)
	      *p = 0;
	    if (*remoteusername && strcmp(remoteusername, identname))
	      syslog(LOG_INFO, "Liar, %s@%s claimed to be %s", identname, host, remoteusername);
	    strcpy(remoteusername, identname);
	  }
	}
	close(s);
      }

      dup2(0, 1);
      dup2(0, 2);
      for (i = 0; i < 4; i++)
      {
	  if (!client)
            execl(BBSEXEC, BBSARG, host, remoteusername, 0);
	  else
	    execl(BBSEXEC, CLIENTARG, host, remoteusername, 0);
        sleep(1);
      }
      send(0, BBSGONE, sizeof BBSGONE - 1, 0);
      shutdown(0, 2);
      sleep(5);
      syslog(LOG_ERR, "Can't exec bbs!");
      _exit(1);
    }
  }

  if (q->oldqp != q->qp)
  {
    q->oldqp = q->qp;
    bigbtmp->queued = q->qp;
  }
  q->qflag = 0;
}
