/*
 * doc_routines.c - General message system code.
 */
#include <stdio_ext.h>
#include "defs.h"
#include "ext.h"



/**********************************************************************
* countmsgs
**********************************************************************/
int
countmsgs(void)
{
int     count = 0;
int     new = 0;
int i;

  for (i = 0; i < MSGSPERRM; i++)
  {
    /* this counts the messages that still exist in main file */
    if (room->num[i] > 0L)
    {
      count++;
      if (room->num[i] > ouruser->lastseen[curr] &&
          (curr != MAIL_RM_NBR || ouruser->mr[MAILMSGS - MSGSPERRM + i].pos > 0))
	new++;
    }
  }

  if (new > 0)
    colorize("@Y[%s]  @C%d @Gmessages,@C %d @Gnew\n", msg->room[curr].name, count, new);

  /* hack to suppress this message when our room contains no posts */
  if (msg->room[curr].descupdate > msg->room[curr].num[MSGSPERRM - 1])
    msg->room[curr].descupdate = msg->room[curr].num[MSGSPERRM - 1];

  if (msg->room[curr].descupdate > ouruser->lastseen[curr])
    my_printf("\n*** Forum description has been updated, hit 'i' to view it. ***\n");
  return(new);
}


/**********************************************************************
* debug report
**********************************************************************/
void
debug(void)
{
}


/**********************************************************************
* knrooms
* List all known rooms with unread messages, list all known rooms
* with unread messages, and list all forgotten rooms.
-----------------------------------------------------------------------*/
void
knrooms(struct user *tmpuser)
{
int     i;
int     limit = 24;
int     linenbr;
int     newlength;
int     oldlength = 1;
char    tmpstr[80];
int     rm_nbr;

  if (checkmail(tmpuser, NOISY) <= 0)
    my_printf("No mail for %s\n", tmpuser->name);

  linenbr = 5;
  colorize("\n   @CForums with unread messages:\n@Y");

  for (rm_nbr = 0; rm_nbr < MAXROOMS; ++rm_nbr)
  {

    if (rm_nbr != MAIL_RM_NBR)
      if ((msg->room[rm_nbr].flags & QR_INUSE)
	  && (msg->room[rm_nbr].highest > tmpuser->lastseen[rm_nbr])
	  && ((rm_nbr != AIDE_RM_NBR)
	      || tmpuser->f_admin)
	  && (msg->room[rm_nbr].gen != tmpuser->forget[rm_nbr])
          && (tmpuser->generation[rm_nbr] != RODSERLING)
          && (tmpuser->forget[rm_nbr] != NEWUSERFORGET)
	  && (((msg->room[rm_nbr].flags & QR_PRIVATE) == NO)
	      || tmpuser->f_prog
	      || (msg->room[rm_nbr].gen == tmpuser->generation[rm_nbr])))
      {

	checked_snprintf(tmpstr, sizeof(tmpstr), " %d\056%s>  ", rm_nbr, msg->room[rm_nbr].name);
	while (strlen(tmpstr) % limit)
	  strcat(tmpstr, " ");

	newlength = oldlength + strlen(tmpstr);

	if (newlength > MARGIN)
	{
	  my_putchar('\n');
	  if (++linenbr >= rows - 1 && line_more(&linenbr, -1))
	    return;
	  oldlength = 1;
	}

	my_printf("%s", tmpstr);
	oldlength = oldlength + strlen(tmpstr);

      }				/* end of monster if */
  }				/* end for loop */

  if (oldlength != 1)		/* finish up last line of this list */
    my_putchar('\n');

  /* Now, want to leave the bottom of the screen blank */
  for (i = linenbr; i < rows - 1; ++i)
  {
    my_putchar('\n');
    if (++linenbr >= rows - 1 && line_more(&linenbr, -1))
      return;
  }
  linenbr = 3;
  oldlength = 1;
  colorize("\n  @C No unseen messages in:@G\n");

  /* now list the rooms that are all read */
  for (rm_nbr = 0; rm_nbr < MAXROOMS; ++rm_nbr)
  {

    if (rm_nbr != MAIL_RM_NBR)
      if ((msg->room[rm_nbr].flags & QR_INUSE)
	  && (msg->room[rm_nbr].highest <= tmpuser->lastseen[rm_nbr])
	  && ((rm_nbr != AIDE_RM_NBR)
	      || tmpuser->f_admin)
	  && (msg->room[rm_nbr].gen != tmpuser->forget[rm_nbr])
          && (tmpuser->generation[rm_nbr] != RODSERLING)
          && (tmpuser->forget[rm_nbr] != NEWUSERFORGET)
	  && (((msg->room[rm_nbr].flags & QR_PRIVATE) == NO)
	      || tmpuser->f_prog
              || (msg->room[rm_nbr].gen == tmpuser->generation[rm_nbr])))
      {

	checked_snprintf(tmpstr, sizeof(tmpstr), " %d\056%s>  ", rm_nbr, msg->room[rm_nbr].name);
	while (strlen(tmpstr) % limit)
	  strcat(tmpstr, " ");
	newlength = oldlength + strlen(tmpstr);

	if (newlength > MARGIN)
	{
	  my_putchar('\n');
	  if (++linenbr >= rows - 1 && line_more(&linenbr, -1))
	    return;
	  oldlength = 1;
	}

	my_printf("%s", tmpstr);
	oldlength = oldlength + strlen(tmpstr);
      }				/* end of monster if */
  }				/* end of for */

  if (oldlength != 1)
    my_putchar('\n');

  /* Now, want to leave the bottom of the screen blank */
  for (i = linenbr; i < rows - 1; ++i)
  {
    my_putchar('\n');
    if (++linenbr >= rows - 1 && line_more(&linenbr, -1))
      return;
  }
  linenbr = 2;
  colorize("\n  @C Forgotten public forums:@G\n");

  /* Zapped room list */
  for (rm_nbr = 0; rm_nbr < MAXROOMS; ++rm_nbr)
  {

    if ((msg->room[rm_nbr].flags & QR_INUSE)
	&& ((msg->room[rm_nbr].gen == tmpuser->forget[rm_nbr])	/* zapped */
            || (tmpuser->forget[rm_nbr] == NEWUSERFORGET)
            || (tmpuser->generation[rm_nbr] == RODSERLING))
	&& ((rm_nbr != AIDE_RM_NBR)
	    || tmpuser->f_admin)
	&& (((msg->room[rm_nbr].flags & QR_PRIVATE) == NO)
	    || tmpuser->f_prog
            || (msg->room[rm_nbr].gen == tmpuser->generation[rm_nbr])))
    {

      checked_snprintf(tmpstr, sizeof(tmpstr), " %d\056%s>  ", rm_nbr, msg->room[rm_nbr].name);
      while (strlen(tmpstr) % limit)
	strcat(tmpstr, " ");
      newlength = oldlength + strlen(tmpstr);

      if (newlength > MARGIN)
      {
	my_putchar('\n');
	if (++linenbr >= rows - 1 && line_more(&linenbr, -1))
	  return;
	oldlength = 1;
      }

      my_printf("%s", tmpstr);
      oldlength = oldlength + strlen(tmpstr);
    }				/* end of monster if */
  }				/* end of for */

  if (oldlength != 1)
    my_putchar('\n');
}			/* end function */


/************************************************************
* line_more
* Increments the linenumber unless noprompt is wanted.  At a 
* screenful, it prompts for space or Q, 
* returns 0 for okay keep going, -1 for quit reading.
************************************************************/
int
line_more(int *nbr, int percent)
{
int     chr;
int     savenox = mybtmp->nox;

  *nbr = 0;
  if (!rows)
    return 0;

  if (client)
  {
    my_putchar(IAC);
    my_putchar(MORE_M);
  }
  for (;;)
  {
    colorize("@Y--MORE--");
    if (percent > 99)
      percent = 99;
    if (percent > -1)
      my_printf("(%d%%) ", percent);
    else
      my_putchar(' ');

    if (!savenox)
      checkx(0);

    if (client)
      for (;;)
      {
        chr = get_single_quiet("0123456789NpPqQSvxY/? \021\n");
        if (chr != 17 || !client || numposts <= 0)
          break;
      }
    else
      chr = get_single_quiet("0123456789NpPqQSvxY/? \n");

    if (strchr("0123456789QxpP?/", chr))
    {
      if (!ouruser)
	continue;
      else if (guest)
      {
        my_printf("\n\nThe Guest user cannot do that.\n\n");
        continue;
      }
      else
        mybtmp->nox = 1;
    }

    switch (chr)
    {
      case SP:
      case 'Y':
        colorize("\r@G              \r");
        if (client)
        {
          my_putchar(IAC);
          my_putchar(MORE_M);
        }
        return(0);

      case LF:
        *nbr = rows - 1; 
        colorize("\r@G              \r");
        if (client)
        {
          my_putchar(IAC);
          my_putchar(MORE_M);
        }
        return(0);

      case 17:	/* ctrl-Q */
      case 'N':
      case 'q':
      case 'S':
        colorize("\r@G              \r");
        if (client)
        {
          my_putchar(IAC);
          my_putchar(MORE_M);
        }
        return(-1);

      case 'p':
      case 'P':
        profile_user(chr == 'P');
        my_putchar('\n');
        break;

      case 'Q':
        get_syself_help(chr);
        my_putchar('\n');
        break;

      case 'v':
        express (-1);
        my_putchar ('\n');
        break;

      case 'x':
        express(10);
        my_putchar('\n');
        break;

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        express (chr - '0');
        my_putchar ('\n');
        break;

      case '?':
      case '/':
        my_printf("\n\nThe help for this section is not yet available.\n");
        my_putchar('\n');
        break;
    }
  }
}


void
flush_input(int sec)
{
int i;
int flush = -1;

  if (sec)
    sleep(sec);
  if (tty)
  {
    //while (INPUT_LEFT())
    //  (void)getchar();
    __fpurge(stdin);
    tcflush(0, TCIFLUSH);
  }
  else
  {
    while ((i = telrcv(&flush)) >= 0)
      if (!block && i != 17)
        byte++;
    if (errno != EWOULDBLOCK)
      my_exit(0);
  }
}


/**********************************************************************
* fr_delete (rmnbr, msgnum, fullrm)
*  Reads the contents of the fullrm structure for a given room #
*  Passes back a new fullrm structure to keep things in sync. 
*  Returns: 0 - message deleted ok.
*           1 - couldn't find message to delete
*
*  int rmnbr - the room #
*  long delnum - the msg # to delete
*  struct fullrm *fullrm - the new updated fullrm (out)
*********************************************************************/
void
fr_delete(long    delnum)
{
int i;

  if (curr == MAIL_RM_NBR)
  {
    /*
     * We only delete mail from our personal mailbox....just like regular US
     * Mail, you can't stop something from arriving once its sent.
     *
     * We have to delete mail from both our fullroom array AND our user file,
     * because our user file is constantly updated as mail is sent to us,
     * while our fullroom array is static for as long as we are in the Mail>
     * room (because we don't reread it when deleting mail -- no reason to do
     * so!)  User file integrity is preserved because the BBS is always locked
     * during mail operations.  After munging the user file, we just fall
     * through into some of the regular delete code, minus the file handling.
     */
    for (i = MAILMSGS - 1; ouruser->mr[i].num != delnum && i >= 0; --i)
      ;
    for (; i > 0; i--)
    {
      ouruser->mr[i].num = ouruser->mr[i - 1].num;
      ouruser->mr[i].pos = ouruser->mr[i - 1].pos;
    }
    if (!i)
      ouruser->mr[0].num = ouruser->mr[0].pos = 0L;
  }
  else
  {
    i = curr == MAIL_RM_NBR ? MAILMSGS - 1 : MSGSPERRM - 1;
    for (i = MSGSPERRM - 1; msg->room[curr].num[i] != delnum && i >= 0; --i)
      ;
    for (; i > 0; i--)
    {
      msg->room[curr].num[i] = msg->room[curr].num[i - 1];
      msg->room[curr].chron[i] = msg->room[curr].chron[i - 1];
      msg->room[curr].pos[i] = msg->room[curr].pos[i - 1];
    }
    if (!i)
      msg->room[curr].num[0] = msg->room[curr].pos[0] = 0L;
  }

  for (i = MSGSPERRM - 1; room->num[i] != delnum && i >= 0; --i)
    ;
  for (; i > 0; i--)
  {
    room->num[i] = room->num[i - 1];
    room->chron[i] = room->chron[i - 1];
    room->pos[i] = room->pos[i - 1];
  }
  if (!i)
    room->num[0] = room->pos[0] = 0L;
}

/********************************************************************
* fr_post (room, msgnum, pos, tmpuser)
*  Reads the contents of the fullrm structure for a given room #
*
*  int rm;
*  long num;                   The eternal number of the message    *
*  long msgnum;                The message # for the room           *
*  long pos;                   Position in the msgmain file         *
*********************************************************************/
void
fr_post(int rm, long msgnum, long pos, long mmhi, struct user *tmpuser)
{
  int i;

  if (rm == MAIL_RM_NBR)
    if (tmpuser)
    {
      for (i = 0; i < MAILMSGS - 1; i++)
      {
        ouruser->mr[i].num = ouruser->mr[i + 1].num;
        ouruser->mr[i].pos = ouruser->mr[i + 1].pos;
        tmpuser->mr[i].num = tmpuser->mr[i + 1].num;
        tmpuser->mr[i].pos = tmpuser->mr[i + 1].pos;
      }
      ouruser->mr[MAILMSGS - 1].num = tmpuser->mr[MAILMSGS - 1].num = mmhi;
      /*
       * your own posts are marked with a negative number so checkmail won't
       * report what you wrote as being new
       */
      ouruser->mr[MAILMSGS - 1].pos = -pos;
      tmpuser->mr[MAILMSGS - 1].pos = pos;
    }
    else
    {
      for (i = 0; i < MAILMSGS - 1; i++)
      {
        ouruser->mr[i].num = ouruser->mr[i + 1].num;
        ouruser->mr[i].pos = ouruser->mr[i + 1].pos;
      }
      ouruser->mr[MAILMSGS - 1].num = mmhi;
      /*
       * your own posts are marked with a negative number so checkmail won't
       * report what you wrote as being new
       */
      ouruser->mr[MAILMSGS - 1].pos = -pos;
    }
  else
  {
    for (i = 0; i < MSGSPERRM - 1; i++)
    {
      msg->room[rm].num[i] = msg->room[rm].num[i + 1];
      msg->room[rm].chron[i] = msg->room[rm].chron[i + 1];
      msg->room[rm].pos[i] = msg->room[rm].pos[i + 1];
    }
    msg->room[rm].num[MSGSPERRM - 1] = msg->room[rm].highest = mmhi;
    msg->room[rm].chron[MSGSPERRM - 1] = msgnum;
    msg->room[rm].pos[MSGSPERRM - 1] = pos;
  }

  if (rm == curr)
  {
    for (i = 0; i < MSGSPERRM - 1; i++)
    {
      room->num[i] = room->num[i + 1];
      room->chron[i] = room->chron[i + 1];
      room->pos[i] = room->pos[i + 1];
    }
    room->num[MSGSPERRM - 1] = room->highest = mmhi;
    room->chron[MSGSPERRM - 1] = rm == MAIL_RM_NBR ? mmhi : msgnum;
    room->pos[MSGSPERRM - 1] = pos;
  }
}



/**********************************************************************
* read room description
**********************************************************************/
void
readdesc(void)
{
int     dummy;
char    name[MAXALIAS + 1];
unsigned char *p;
int size;

  char * file = my_sprintf("%sroom%d", DESCDIR, curr);
  size = 0;
  if (!(p = (unsigned char *)mymmap(file, &size, 0)) || !size)
  {
    colorize("@RNo Forum Info is available\n");
    if (p)
      munmap((void *)p, size);
    free(file);
    return;
  }
  free(file);

  readmessage(p, &dummy, name, FALSE, 0);

  munmap((void *)p, size);
}


/**************************************************************************
* storeug
* Store <u>ngoto information in prev_rm
* do an <U>ngoto prev_rm and save the universal message number of the message
* last seen in *uglastmsg.
***************************************************************************/
void
storeug(long   *uglastmsg, long   *ugtemp)
{
  *uglastmsg = *ugtemp;
  *ugtemp = ouruser->lastseen[curr];
}



/**********************************************************************
* ungoto
**********************************************************************/
void
ungoto(int prev, long *uglastmsg, long *ugtemp)
{
  if (prev == TWILIGHTZONE)
    return;

  ouruser->lastseen[curr] = *ugtemp;
  curr = prev;
  ouruser->lastseen[curr] = *uglastmsg;
  *ugtemp = *uglastmsg;

  openroom();
}


/********************************************************************
* updatels
* update last seen: make all messages old in current room.
* Notate ouruser record accordingly.
**********************************************************************/
void
updatels(short    *prev)
{
  *prev = curr;
  ouruser->lastseen[curr] = room->num[MSGSPERRM - 1];
}



/**********************************************************************
* yesno
* Waits for a keypress and outputs Yes or No, 
* returns Y == YES , N == NO
**********************************************************************/
int
yesno(int def)
{
int i;
 
  if (def < 0)
    i = get_single_quiet("YN");
  else
  {
    my_printf("(%s) ", def ? "Yes" : "No");
    i = get_single_quiet(" \nYN");
    if (i == '\n' || i == ' ')
      i = def ? 'Y' : 'N';
  }
  if (i == 'Y')
  {
    my_printf("Yes\n");
    return(YES);
  }
  my_printf("No\n");
  return(NO);
}

void 
do_fortune(void)
{
char 	*cmd = FORTUNE;
char 	*p;
char 	buf[BUFSIZ];
FILE 	*fp;

  if ((fp = popen (cmd, "r")) == NULL)
    return;

  while (fgets (buf, BUFSIZ, fp) != NULL)
  {
    if ((p = rindex (buf, '\n')) != NULL)
      *p = 0;
    my_printf ("%s\n", buf);
  }
  pclose (fp);
}


/*
char *months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
*/

char *days[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
                              


char *
formtime (int how, time_t timetoform)
{
static char tstring[80];
struct tm *tm;
int hr;		/* AM/PM conversion */
const char * stamp = "";

  tm = localtime (&timetoform);

  if (ouruser->f_ampm)
  {
    hr = !tm->tm_hour ? 12 :
    	 tm->tm_hour > 12 ? tm->tm_hour - 12 : tm->tm_hour;
    stamp = tm->tm_hour >= 12 ? " PM" : " AM";
  }
  else
    hr = tm->tm_hour;

  switch (how)
  {
    case 1:
      /* Mon Jan 1, 2000  1:23 PM */
      checked_snprintf (tstring, sizeof(tstring), "%s %s %d, %d %d:%02d%s",
      	days[tm->tm_wday], months[tm->tm_mon], tm->tm_mday,
      	1900 + tm->tm_year, hr, tm->tm_min, stamp);
      break;

    case 2:
      /* Jan 1, 2000  1:23 PM */
      checked_snprintf (tstring, sizeof(tstring), "%s %d, %d %d:%02d%s",
	months[tm->tm_mon], tm->tm_mday, 1900 + tm->tm_year,
	hr, tm->tm_min, stamp);
      break;

    case 3:
      /* 1/1/00  1:23 PM */
      checked_snprintf (tstring, sizeof(tstring), "%d/%d/%02d %d:%02d%s",
	tm->tm_mon + 1, tm->tm_mday, tm->tm_year % 100,
	hr, tm->tm_min, stamp);
      break;

    case 4:
      /* 1:23 PM */
      checked_snprintf (tstring, sizeof(tstring), "%d:%02d%s", hr, tm->tm_min, stamp);
      break;

    case 5:
      /* Jan 1, 2000 */
      checked_snprintf (tstring, sizeof(tstring), "%s %d, %d",
	months[tm->tm_mon], tm->tm_mday, 1900 + tm->tm_year);
      break;

    case 6:		/* X message */
      checked_snprintf (tstring, sizeof(tstring), "%d:%02d%s on %s %d, %d",
	hr, tm->tm_min, stamp, months[tm->tm_mon], tm->tm_mday,
	1900 + tm->tm_year);
      break;

    default:
      my_printf ("Unknown error in formtime\n");
      break;
  }
  return (tstring);
}


#include <utmpx.h>
char *gethost(void)
{
struct utmpx *ut;
char *tp;
static char hname[MAXHOSTNAMELEN + 1];

  if (*hname)
    return (hname);

  /* Take our argument's word over anyone else's */
  if (ARGV[1])
    checked_snprintf (hname, sizeof(hname), "%s", ARGV[1]);
  else
  {
    if ((tp = ttyname (0)) == NULL)
      return NULL;

    /* So outside buttheads don't 'talk bbs' */
    chmod (tp, 0600);
    tp = (char *) index (tp, '/') + 5;
    checked_snprintf(hname, sizeof(hname), "%s", tp);

    /* Not all systems have this, so you may have to roll your own... */

    setutxent();
    while ((ut = getutxent()) != NULL)
      if (  !strcmp (hname, ut->ut_line) 
            && ut->ut_pid == pid && *ut->ut_host  )
      {
        checked_snprintf(hname, sizeof(hname), "%s", ut->ut_host);
	break;
      }
    endutxent();

  }

  return (hname);
}

/*
 * a case insensitive strstr().
 */
const char *
mystrstr(const char *haystack, const char* needle)
{
    const size_t nlen = strlen(needle);

    const char *s;
    for (s = haystack; *s; s++)
        if ( /* *s == *needle && */ !strncasecmp(s, needle, nlen))
            break;

    if (!*s)
        return (char *) NULL;
    else
        return s;
}

char *
mystrstr_nonconst(char *haystack, const char* needle)
{
    return (char*)mystrstr(haystack, needle);
}

void
bcdplus1encode (long number)
{
char	digit;
long	remainder;

  if (number < 0)
    return;

  remainder = number / 10;
  digit = number - remainder * 10 + 1;

  if (remainder > 0)
    bcdplus1encode (remainder);

  my_putc (digit, stdout);
}

void
version(void)
{
  my_printf ("#define BBSNAME		\"%s\"\n", BBSNAME);
  my_printf ("#define BBSUID		%d\n", BBSUID);
  my_printf ("#define BBSGID		%d\n", BBSGID);
  my_printf ("#define MAILMSGS		%d\n", MAILMSGS);
  my_printf ("#define MSGSPERRM		%d\n", MSGSPERRM);
  my_printf ("#define MAXROOMS		%d\n", MAXROOMS);
  my_printf ("#define MM_FILELEN	%d\n", MM_FILELEN);
  my_printf ("#define ROOT		%s\n", ROOT);
  my_printf ("#define MAXTOTALUSERS	%d\n", MAXTOTALUSERS);
}

