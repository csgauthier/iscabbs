/*
 * Syncs the major BBS files and maintains the idle file.
 * Need to fix the 61036*4096 kludge...
 */
#include "defs.h"
#include "ext.h"



int
bbssync(int init)
{
  long pst_data[32];
  unsigned char outbuf[100];
  char s[100];
  struct pst_dynamic *pst = (void *)pst_data;
  long oldcputime[9];
  time_t t;
  int flags;
  long curpos;
  FILE *fp;
  int f;
  struct tm *tm;
  int i;
  int trim;

  if (fork())
    _exit(0);
  close(0);
  close(1);
  close(2);
  setsid();
#ifdef __hpux
  signal(SIGCLD, SIG_IGN);
#endif

  if (!(fp = fopen(ROOT"/var/idle", "a")))
    _exit(0);
  setvbuf(fp, outbuf, _IOFBF, sizeof outbuf);
  flags = fcntl(fileno(fp), F_GETFL);
  flags |= O_SYNC;
  fcntl(fileno(fp), F_SETFL, flags);

  if (init)
  {
    bzero((void *)bigbtmp, sizeof(struct bigbtmp));
    for (i = 0; i < 7; i++)
      msem_init(&msg->sem[i], -1);

    if (!fork())
    {
      setsid();
#if 1
      execl(BBSEXEC, BBSEXEC, "-f", NULL);
#endif
      return(FINGER);
    }

    if (!fork())
    {
      setsid();
      sleep(60);
#if 1
      execl(BBSEXEC, BBSEXEC, "-q", NULL);
#endif
      return(QUEUE);
    }
  }

  nice(-20);
  nice(-20);
#ifdef __hpux
  rtprio(0, 64);
#endif

  for (oldcputime[0] = 0;;)
  {
    t = msg->t = time(0);
    tm = localtime(&t);
    sleep(60 - tm->tm_sec);

    t = msg->t = time(0);
    tm = localtime(&t);
    /* pstat_getdynamic(pst, sizeof pst_data, 0, 0); */

    if (oldcputime[0])
    {
//      fprintf(fp, "%02d/%02d %02d:%02d  %4d/%3d  user %02d%%/%02d%%  sys %02d%%  idle %02d%%/%02d%%  %5.2f/%5.2f/%5.2f\n", tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, bigbtmp->users, bigbtmp->queued, (pst->psd_cpu_time[0] - oldcputime[0]) / 60, (pst->psd_cpu_time[1] - oldcputime[1]) / 60, (pst->psd_cpu_time[2] - oldcputime[2]) / 60, (pst->psd_cpu_time[3] - oldcputime[3]) / 60, (pst->psd_cpu_time[4] - oldcputime[4]) / 60, pst->psd_avg_1_min, pst->psd_avg_5_min, pst->psd_avg_15_min);
      fflush(fp);
    }
//    for (f = 0; f < 5; f++)
 //     oldcputime[f] = pst->psd_cpu_time[f];

    /* Busy rates are 120K/min X messages, 8K/min messages */
    /* This is 32 msgs/sec, ??? xmsgs/sec */
    /* Sync with INVALID the unused sections of the xmsg file somewhere? */

#if 0
    if (tm->tm_min % 3 == 0)
      sync();
#endif

    /* Here, should only sync in the area of curpos! */
    if (tm->tm_min % 15 == 5)
      msync((caddr_t)msgstart, 61036*4096, MS_ASYNC);

    /* Break this one up and save in pieces?  Need to maintain it in a
     * consistent state.  If make sure forums are on page boundaries, it will
     * work -- save the first few pages of crud one minute, then x forums each
     * minute after that, saving all over y minutes */
    if (tm->tm_min % 15 == 10)
      msync((caddr_t)msg, sizeof(struct msg), MS_ASYNC);

    trim = 4;
    curpos = msg->xcurpos & ~4095;
    if (curpos - trim*1024*1024 >= 0)
    {
      if (curpos + 256*1024 >= msg->xmsgsize) 
        msync((caddr_t)xmsg + (curpos + 256*1024 - msg->xmsgsize), curpos - trim*1024*1024 - (curpos + 256*1024 - msg->xmsgsize), MS_INVALIDATE);
      else
        msync((caddr_t)xmsg, curpos - trim*1024*1024, MS_INVALIDATE);
    }
    if (curpos + 256*1024 < msg->xmsgsize)
    {
      if (trim*1024*1024 - curpos >= 0)
        msync((caddr_t)xmsg + curpos + 256*1024, msg->xmsgsize - curpos - 256*1024 - (trim*1024*1024 - curpos), MS_INVALIDATE);
      else
        msync((caddr_t)xmsg + curpos + 256*1024, msg->xmsgsize - curpos - 256*1024, MS_INVALIDATE);
    }

    if (tm->tm_min % 15 == 0)
      trim = 8;
    else
      trim = 16;

    curpos = msg->curpos & ~4095;
    if (curpos - trim*1024*1024 >= 0)
    {
      if (curpos + 256*1024 >= 61036*4096)
        msync((caddr_t)msgstart + (curpos + 256*1024 - 61036*4096), curpos - trim*1024*1024 - (curpos + 256*1024 - 61036*4096), MS_INVALIDATE);
      else
        msync((caddr_t)msgstart, curpos - trim*1024*1024, MS_INVALIDATE);
    }

    if (curpos + 256*1024 < 61036*4096)
    {
      if (trim*1024*1024 - curpos >= 0)
        msync((caddr_t)msgstart + curpos + 256*1024, 61036*4096 - curpos - 256*1024 - (trim*1024*1024 - curpos), MS_INVALIDATE);
      else
        msync((caddr_t)msgstart + curpos + 256*1024, 61036*4096 - curpos - 256*1024, MS_INVALIDATE);
    }

    {
    char * corepath = my_sprintf(ROOT"/core/bbs/core-%02d%02d%02d%02d",
                      tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min);
    rename("/bbs/core/bbs/bbs.core", corepath);
    free(corepath);
    }

    {
    char * corepath = my_sprintf(ROOT"/core/bbsqueued/core-%02d%02d%02d%02d",
                      tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min);
    rename("/bbs/core/bbsqueued/bbs.core", corepath);
    free(corepath);
    }
  }
}
