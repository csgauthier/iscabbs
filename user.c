/*
 * user.c - Handles user list setup/updates.
 */
#include "defs.h"
#include "ext.h"

static void newmaxnewbie(int num);

void
locks(int which)
{
  int i;

  if (lockflags)
  {
    errlog("Tried to lock twice on flags %d & %d", lockflags, which);
    raise(SIGABRT);
  }

  if ((lockflags = which) < 0)
    return;

  while ((i = msem_lock(&msg->sem[which], 0)) < 0 && errno == EINTR) 
    if (f_death == 2)
      break;
  if (f_death == 2)
  {
    lockflags = 0;
    my_exit(0);
  }

  if (i < 0)
  {
    errlog("Got error %d when locking %d", errno, which);
    raise(SIGABRT);
  }
}
 

void
unlocks(int which)
{
  int i;

  if (which < 0)
  {
    lockflags = 0;
    return;
  }

  while ((i = msem_unlock(&msg->sem[which], 0)) < 0 && errno == EINTR)
    if (f_death == 2)
      break;
  if (f_death == 2)
  {
    errlog("Timed out when unlocking %d", which);
    raise(SIGABRT);
  }

  if (i < 0 && errno != EAGAIN)
  {
    errlog("Got error %d when unlocking %d", errno, which);
    perror ("huh?");
    raise(SIGABRT);
  }

  lockflags = 0;
}



int
add_loggedin(struct user *up)
{
int i;
int j;
int p;
int count = 0;
int btmpindex = 0;


  for (;;)
  {
    if ((btmpindex = ouruser->btmpindex) >= 0)
    {
      locks(SEM_USER);
      if (!bigbtmp->btmp[btmpindex].pid || bigbtmp->btmp[btmpindex].usernum != ouruser->usernum)
        ouruser->btmpindex = -1;
      unlocks(SEM_USER);
    }
      
    locks(SEM_BTMP);
    for (i = 0; i < MAXUSERS; i++)
      if ((p = bigbtmp->btmp[i].pid) && bigbtmp->btmp[i].usernum == up->usernum)
        if (!guest)
        {
          unlocks(SEM_BTMP);
	  if (kill(p, 0) < 0)
	  {
	    errlog("Removed ghost login pid %d index %d", p, i);
	    remove_loggedin(p);
	    locks(SEM_USER);
	    if (ouruser->btmpindex == i)
              ouruser->btmpindex = -1;
	    unlocks(SEM_USER);
	    break;			/* Breaks out to for (;;) to restart */
	  }
          my_printf("\n\aLogging off previous login under your username");
	  fflush(stdout);
	  for (j = 0; j <= 15 && bigbtmp->btmp[i].pid == p && bigbtmp->btmp[i].usernum == up->usernum; j++)
	  {
	    if (j == 0)
	      kill(p, SIGQUIT);
	    else if (j == 3)
	      kill(p, SIGTERM);
	    else if (j == 6)
	    {
	      if (kill(p, SIGABRT) < 0)
	        errlog("Error %s forcing SIGABRT pid %d index %d", strerror(errno), p, i);
	      else
		errlog("Forced SIGABRT pid %d index %d", p, i);
	      remove_loggedin(p);
	      locks(SEM_USER);
	      if (ouruser->btmpindex == i)
                ouruser->btmpindex = -1;
	      unlocks(SEM_USER);
	    }
	    my_putchar('.');
	    sleep(2);
	    if (kill(p, 0) < 0)
	      break;
	  }
	  my_putchar('\n');
	  my_putchar('\n');
	  break;			/* Breaks out to for (;;) to restart */
        }
        else
          count++;
      else if (&bigbtmp->btmp[i] == mybtmp)
        btmpindex = i;

    if (i < MAXUSERS || ouruser->btmpindex >= 0)
    {
      unlocks(SEM_BTMP);
      continue;
    }

    mybtmp->xstat = up->f_xoff;
    mybtmp->elf = up->f_autoelf && up->f_elf && !up->f_restricted && !up->f_twit;
    mybtmp->connecting = 0;
    mybtmp->guest = guest;
    mybtmp->client = client;
    mybtmp->usernum = up->usernum;
    mybtmp->ulink = getuserlink(up);
    strcpy(mybtmp->name, up->name);
    doingchange (ouruser->doing);
    unlocks(SEM_BTMP);
    ouruser->time = mybtmp->time;
    locks(SEM_USER);
    if (ouruser->btmpindex == -1)
      ouruser->btmpindex = btmpindex;
    else
    {
      unlocks(SEM_USER);
      errlog("btmpindex %d != ouruser->btmpindex %d", btmpindex, ouruser->btmpindex);
      ouruser->btmpindex = 0;
      // mybtmp->pid = 0;
      continue;
    }
    unlocks(SEM_USER);

    if (count >= (MAXUSERS >> 5))
    {
      my_printf("\nI'm sorry, too many Guest users are logged in at the moment, please try again\nlater.\n\n");
      my_exit(10);
    }

    return(bigbtmp->users);
  }
}


void
remove_loggedin(int p)
{
int i;
struct btmp *btmp;

  for (i = bigbtmp->users - 1; i >= 0; i--)
    if (bigbtmp->btmp[bigbtmp->index[i]].pid == p)
    {
      locks(SEM_BTMP);
      if (bigbtmp->btmp[bigbtmp->index[i]].pid == p)
      {
	for (btmp = &bigbtmp->btmp[bigbtmp->index[i]]; i < bigbtmp->users - 1; i++)
	  bigbtmp->index[i] = bigbtmp->index[i + 1];
	(bigbtmp->users)--;
	btmp->pid = 0;
	btmp->eternal = ++(bigbtmp->eternal);
	unlocks(SEM_BTMP);
	break;
      }
      else
        i = bigbtmp->users;
      unlocks(SEM_BTMP);
    }
}


/*
 * reserve_slot() 
 * Reserves a slot on the online list 
 */

void
reserve_slot(void)
{
struct btmp newbtmp;
int i;
pid_t p;
int j;
struct sockaddr_in sa;
long mineternal;

  /* Set up a place holder in the btmp file so no one else gets this spot */
  bzero((void *)&newbtmp, sizeof newbtmp);
  strcpy(newbtmp.name, "<Connecting>");
  strcpy(newbtmp.doing, "Logging in...");
  newbtmp.connecting = 1;
  newbtmp.usernum = 0;
  time(&newbtmp.time);
  /* 23 spaces in the wholist for the hostname */
  strncpy(newbtmp.remote, gethost(), 23);
  newbtmp.xstat = 1;
  newbtmp.nox = 1;
  newbtmp.pid = pid;
  if (tty)
  {
    newbtmp.remaddr = 0;
    newbtmp.remport = 0;
    *newbtmp.remlogin = 0;
  }
  else
  {
    j = sizeof(sa);
    getpeername(0, (struct sockaddr*)&sa, &j);
    newbtmp.remaddr = sa.sin_addr.s_addr;
    newbtmp.remport = sa.sin_port;
    strcpy(newbtmp.remlogin, ARGV[1] && ARGV[2] ? ARGV[2] : "");
  }

  /* used to check user limits here...now just check for file overflow */
  if (bigbtmp->users >= MAXUSERS - 1)
  {
    errlog("Who list overflow at %ld users", bigbtmp->users);
    my_exit(0);
  }

  if (bigbtmp->ghostcheck + 60 < newbtmp.time)
  {
    bigbtmp->ghostcheck = newbtmp.time;
    for (i = 0; i < MAXUSERS; i++)
      if ((p = bigbtmp->btmp[i].pid))
	if (kill(p, 0) < 0)
	{
	  errlog("Cleaned up ghost login of %s: %s", bigbtmp->btmp[i].name, strerror(errno));
	  remove_loggedin(p);
	  if (bigbtmp->btmp[i].pid == p)
	  {
	    bigbtmp->btmp[i].pid = 0;
	    errlog("Cleaned up bogus ghost login of %s", bigbtmp->btmp[i].name);
	  }
	}
/*
        else if (bigbtmp->btmp[i].time + 242 * 60 < newbtmp.time)
        {
	  errlog("Aborted ghost login of %s", bigbtmp->btmp[i].name);
	  kill(p, SIGABRT);
          remove_loggedin(p);
        }
*/
  }

  for (;;)
  {
    for (mineternal = 999999999, i = 0; i < MAXUSERS; i++)
      if (!bigbtmp->btmp[i].pid && bigbtmp->btmp[i].eternal < mineternal)
        mineternal = bigbtmp->btmp[p = i].eternal;
    locks(SEM_BTMP);
    if (!bigbtmp->btmp[p].pid)
    {
      newbtmp.eternal = ++(bigbtmp->eternal);
      bigbtmp->btmp[p] = newbtmp;
      bigbtmp->index[bigbtmp->users] = p;
      if (++bigbtmp->users > msg->maxusers)
        msg->maxusers = bigbtmp->users;
      errlog("Reserving btmp slot %d for pid %d (eternal %ld)", p, bigbtmp->btmp[p].pid, bigbtmp->eternal);
      unlocks(SEM_BTMP);
      break;
    }
    unlocks(SEM_BTMP);
  }
  mybtmp = &bigbtmp->btmp[p];
}



void
clientwho(void)
{
struct btmp *btmp;
int i;
time_t t;
time_t ontime;
long mineternal;


  if (!client)
    return;
  t = time(0);
  if (t - lastwho < 150)
  {
    my_putchar(IAC);
    my_putchar(S_WHO);
    my_putchar(0);
    my_putchar(0);
    return;
  }
  lastwho = t;
  my_putchar(IAC);
  my_putchar(S_WHO);
  for (mineternal = 999999999, i = bigbtmp->users - 1; i >= 0; i--)
  {
    btmp = &bigbtmp->btmp[bigbtmp->index[i]];
    if (!btmp->pid || btmp->eternal >= mineternal)
      continue;
    mineternal = btmp->eternal;
    if (!btmp->connecting && !btmp->guest)
    {
      ontime = (t - btmp->time) / 60;
      
      if (ontime >= 254)
      {
        my_putchar (0xfe);
        bcdplus1encode (ontime);
        my_putchar (0);

	/* I really have no idea what's wrong in the client that makes
	 * me reprint that first char.  It worked just fine under ABC...
	 * fuck it, I don't really care.  It works.
	 */

	if (btmp->xstat)
	  my_putchar (*btmp->name | 0x80);
	else
	  my_putchar (*btmp->name);

      } else
	my_putc(ontime + 1, stdout);

      if (btmp->xstat)
        my_putchar(*btmp->name | 0x80);
      else
        my_putchar(*btmp->name);
      fputs(btmp->name + 1, stdout);
      my_putchar(0);
    }
  }
  my_putchar(0);
}



void
validate_users(int what)
{
  int c = ' ';
  int i;
  int count;
  int pick;
  time_t t;
  time_t oldt;
  struct user *tuser = NULL;
  struct btmp btmp;
  unsigned char skips[(MAXNEWBIES >> 3) + 1];
  short skipped = 0;

  if (what)
    bzero((void *)skips, sizeof skips);
  while (c != 'Q')
  {
    t = time(0);
    for (i = 0, count = 0, pick = -1; i <= msg->maxnewbie; i++)
      if (msg->newbies[i].time && msg->newbies[i].time + 14400 < t && ++count && !(skips[i >> 3] & 1 << (i & 7)) && (pick < 0 || msg->newbies[pick].time > msg->newbies[i].time))
        pick = i;
    if (!count)
    {
      my_printf("\nThere are no new users to verify.\n");
      return;
    }
    if (!what)
    {
      my_printf("\nThere are %d new users to verify.\n", count);
      return;
    }
    if (pick < 0)
    {
      my_printf("\n(The remaining %d users were skipped)\n", skipped);
      return;
    }
    locks(SEM_NEWBIE);
    if (msg->newbies[pick].time && msg->newbies[pick].time + 14400 < t)
    {
      i = pick;
      oldt = msg->newbies[i].time;
      msg->newbies[i].time = t;
      unlocks(SEM_NEWBIE);
      my_printf("\nThere are %d users remaining, %d of those have been skipped.\n", count, skipped);
    }
    else
    {
      unlocks(SEM_NEWBIE);
      continue;
    }

    for (c = 'p';;)
    {
      switch (c)
      {
	case '?':
	  my_printf ("\n\nChoices are <A>ccept <C>hange <D>elete <I>gnore <P>rofile <S>kip <Q>uit\n");
	  break;

	case 'A':
	  my_printf("Accept\n");
	  tuser->f_newbie = 0;
	  do_verify(tuser, 0);
	  msg->newbies[i].time = 0;
          if (i == msg->maxnewbie)
            newmaxnewbie(i);
	  ouruser->vals++;
	  break;

	case 'I':
	  my_printf("Ignore  (Bad info flag set unless deleted or inactive)\n");
	  if (tuser->f_trouble)
	  {
	    my_printf("\nA 'Trouble' user must be either deleted, accepted, or skipped.\n");
	    c = 0;
	    break;
	  }
	  if (!tuser->f_invisible)
	  {
	    locks(SEM_USER);
	    tuser->f_newbie = tuser->f_badinfo = tuser->f_restricted = 1;
	    unlocks(SEM_USER);
	  }
	  msg->newbies[i].time = 0;
          if (i == msg->maxnewbie)
            newmaxnewbie(i);
	  ouruser->vals++;
	  break;

	case 'D':
	  my_printf("Delete\n");
	  locks(SEM_USER);
	  tuser->f_deleted = tuser->f_invisible = 1;
	  unlocks(SEM_USER);
	  if (is_online(&btmp, tuser, NULL))
	    logout_user(tuser, NULL, 0);
	  msg->newbies[i].time = 0;
          if (i == msg->maxnewbie)
            newmaxnewbie(i);
	  ouruser->vals++;
	  break;

	case 'C':
	  my_printf("Change\n");
	  change_setup(tuser);
	  break;

	case 'P':
	  my_printf("Profile\n");
      // fall through
	case 'p':
	  if (!tuser && !(tuser = getuser(msg->newbies[i].name)))
	  {
	    c = ' ';
	    msg->newbies[i].time = 0;
            if (i == msg->maxnewbie)
              newmaxnewbie(i);
	    my_printf("\nWARNING:  User '%s' listed, user file not found!\n", msg->newbies[i].name);
	  }
	  else
	    profile(NULL, tuser, PROF_ALL);
	  break;

	case 'Q':
	  my_printf("Quit\n");
	  msg->newbies[i].time = oldt;
	  break;

	case 'S':
	  my_printf("Skip\n");
	  msg->newbies[i].time = oldt;
	  skips[i >> 3] |= 1 << (i & 7);
	  skipped++;
	  break;
      }
      if (c == ' ' || c == 'A' || c == 'D' || c == 'I' || c == 'S' || c == 'Q')
        break;
      my_printf ("\nAction on user %s> ", msg->newbies[i].name);
      c = get_single_quiet("ACDIPSQ?");
    }
    if (tuser)
    {
      freeuser(tuser);
      tuser = NULL;
    }
  }
}

static void
newmaxnewbie(int num)
{
  locks(SEM_NEWBIE);
  if (num == msg->maxnewbie)
  {
    while(num && !msg->newbies[--num].time)
      ;
    msg->maxnewbie = num;
  }
  unlocks(SEM_NEWBIE);
}



/*
 * Logs off specified user.  If btmp entry is given, pid is taken from there
 * and signal is sent according to how, with no checking of success or any
 * notification done (used for logging off all users)  If tmpuser entry is
 * given, progress messages are provided.
 */
void
logout_user(struct user *tmpuser, struct btmp *btmp, int how)
{
  struct btmp copybtmp;
  int copyindex;
  int i;

  if (btmp)
  {
    if ((i = btmp->pid))
      kill(i, how ? SIGTERM : SIGQUIT);
    return;
  }

  for (;;)
  {
    copyindex = tmpuser->btmpindex;
    if (!is_online(&copybtmp, tmpuser, NULL))
      return;
    if (copyindex == tmpuser->btmpindex)
      break;
  }

  my_printf("\nLogging off %s", tmpuser->name);
  kill(copybtmp.pid, SIGQUIT);
  for (i = 0; i < 10 && tmpuser->btmpindex == copyindex; i++)
  {
    my_putchar('.');
    sleep(2);
  }
  if (i == 10)
    kill(copybtmp.pid, SIGTERM);
  my_putchar('\n');
}
