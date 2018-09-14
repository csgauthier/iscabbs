/*
 * doc_msgs.c - Basic handling of messages: read, post, delete.
 */
#include "defs.h"
#include "ext.h"


/****************************************************************************
* deletemessage                  Updated 10/27/91 -dll
* Deletes message from current room, delnum is the universal message nbr of
* the notes to be nuked.
* The fullroom and quickroom structures are local because this routine
* reads the files and updates them itself.
* Adjusted for FRchron on 4-23-91 -dn
****************************************************************************/
void
deletemessage(long delnum, int quiet)
{
  locks(SEM_MSG);
  fr_delete(delnum);
  msg->room[curr].highest = msg->room[curr].num[MSGSPERRM - 1];
  unlocks(SEM_MSG);
  if (!quiet)
    colorize("@RMessage deleted@G\n");
}


/*******************************************************************
* enter message
* This does what the code labelled ENTMSG:, SKFALL:, ENTFIN: in Cit did,
* (and more!).
*
* Terminology:
* 'Post' refers to a public room message.
* 'Letter' refers to a private mail message.
* Letters to Sysop are actually treated as posts to aideroom.
* Twit posts are diverted to the twitroom which is usually aideroom.
* When message forwarding is implemented, this will have to be redone.
*
* This uses an alternate method of keeping mail.
*
**********************************************************************/
int
entermessage(int     troom,
  char   *recipient,
  int     upload)
{
int     curr_rm = troom;
int     err;
int     mtype = MES_NORMAL;	/* message type; i.e., anon, etc. */
long    mmpos;
long    mmhi;
struct user *tmpuser = NULL;
int     tosysop = NO;	/* message to sysop flag */
int     len;
struct mheader *mh;
int i;


  if (curr_rm == LOBBY_RM_NBR && !ouruser->f_admin)
  {
    colorize("@RYou can't post messages here!\n@G");
    return(FALSE);
  }
  else if (curr_rm == LOBBY_RM_NBR)
  {
    my_printf("Are you sure you want to post to the Lobby? (Y/N) -> ");
    if (!yesno(-1))
      return(FALSE);
    my_putchar('\n');
  }

  if (ouruser->f_twit && troom >= 0)
  {
    my_printf("Sorry, you may not post messages while your BBS privileges are suspended.\nPlease Yell to the sysops for more information.\n");
    return(FALSE);
  }

  if (upload == 1)
    my_printf("You are uploading this message! (Use ctrl-D to end)\n\n");
  else if (upload == 2)
  {
    if (troom < 0)
      my_printf("\nHit Y to upload a Yell or N to enter a Yell normally (Y/N) -> ");
    else
      my_printf("Hit Y to upload a message or N to enter a message normally (Y/N) -> ");
    if (yesno(-1) == YES)
    {
      my_printf("\n(Use control-D to end!)\n\n");
      upload = 1;
    }
    else
    {
      my_printf("\n(Enter %s normally)\n\n", troom < 0 ? "Yell" : "message");
      upload = 0;
    }
  }
  else if (troom < 0)
    my_putchar('\n');

  /* If this is email (in Mail>) we need a recipient */
  /* If <R>eply was called, recipient should already be set */
  if (((curr_rm == MAIL_RM_NBR || curr_rm == AIDE_RM_NBR) && (*recipient || curr_rm == MAIL_RM_NBR)) || upload < 0)
  {
    if (upload < 0)
      recipient = get_name("This command is intended to send harassing X messages you have received to the\nSysops, please hit return if you do not wish to do this.  Otherwise, enter the\nname of the user whose X's you want to send: ", 2);
    else if (!*recipient)
      recipient = get_name("Enter recipient: ", 2);
    else if (sysopflags & SYSOP_FROM_USER)
      mtype = MES_SYSOP;

    if (!*recipient)
      return(FALSE);

    if (!strcmp(recipient, "Guest"))
    {
      my_printf("\nThat user cannot receive mail.\n");
      return(FALSE);
    }
 
    if (!strcmp(recipient, "Sysop") && upload >= 0)
    {
      my_printf("\nUse the <Y>ell command to contact the sysops.\n");
      return(FALSE);
    }

    if (curr_rm == AIDE_RM_NBR)
      curr_rm = MAIL_RM_NBR;
  }				/* end if mailroom (recipient analysis) */
  if (*recipient)
    if (!(tmpuser = getuser(recipient)) || tmpuser->f_invisible)
    {
      if (tmpuser)
	freeuser(tmpuser);
      my_printf("\nThere is no user %s on this BBS.\n", recipient);
      flush_input(1);
      return(FALSE);
    }
    else if (tmpuser->f_twit && !ouruser->f_admin && upload >= 0)
    {
      freeuser(tmpuser);
      my_printf("\nThat user has been twitted and cannot receive mail!\n");
      flush_input(1);
      return(FALSE);
    }
    else
    {
      for (i = 0; i < NXCONF && (tmpuser->xconf[i].usernum != ouruser->usernum || tmpuser->xconf[i].which); i++)
        ;
      if (i < NXCONF && upload >= 0)
      {
        my_printf("\nSorry, %s has you on his/her disable list.\n", tmpuser->name);
        freeuser(tmpuser);
        flush_input(1);
        return(FALSE);
      }
      for (i = 0; i < NXCONF && (ouruser->xconf[i].usernum != tmpuser->usernum || ouruser->xconf[i].which); i++)
        ;
      if (i < NXCONF && upload >= 0)
      {
        my_printf("\nSorry, you have %s on your disable list.\n", tmpuser->name);
        freeuser(tmpuser);
        flush_input(1);
        return(FALSE);
      }
      curr_rm = MAIL_RM_NBR;
    }

  if (sysopflags & SYSOP_FROM_SYSOP)
    mtype = MES_SYSOP;
  if (sysopflags & SYSOP_FROM_FM)
    mtype = MES_FM;

  if (curr_rm < 0 || upload < 0)
  {
    tosysop = TRUE;
    curr_rm = AIDE_RM_NBR;
  }
  if (upload < 0)
    mtype = MES_XYELL;

  /* Set the room type */
  if (!*recipient && !tosysop && mtype != MES_SYSOP && mtype != MES_FM && (msg->room[curr_rm].flags & QR_ANONONLY))
    mtype = MES_ANON;

  if (!*recipient && !tosysop && (msg->room[curr_rm].flags & QR_ANON2) && !ouruser->f_noanon && mtype != MES_SYSOP && mtype != MES_FM)
  {
    my_printf("Anonymous? (Y/N) -> ");
    if (yesno(-1) == YES)
    {
      mtype = MES_AN2;
      my_printf("\nIt might be helpful if you included an alias for yourself in your anonymous\npost somewhere, to allow those who respond to you to identify you in some\nfashion.  It is not required, but it is encouraged to reduce confusion.\n\n");
    }
    else
      my_putchar('\n');
  }

  if ((msg->room[curr_rm].flags & QR_ANONONLY) && ouruser->f_noanon)
  {
    colorize("@RYou are not permitted to post in an anon-only forum.\n@G");
    if (tmpuser)
      freeuser(tmpuser);
    return(FALSE);
  }

  /*
   * Now go and enter the message.  The message is created in a temp file and
   * will be integrated into the message database later     
   */

  err = makemessage(tmpuser, mtype, upload);

  if (err == ABORT)
  {
    munmap((void *)tmpstart, 53248);
    if (mtype == MES_XYELL)
      colorize("@RNo X's to you from that user.\n@G");
    else
      colorize("@RMessage not entered\n@G");
    if (tmpuser)
      freeuser(tmpuser);
    return(FALSE);
  }
  else if (err != SAVE)
  {
    if (err != MMAP_ERR)
      munmap((void *)tmpstart, 53248);
    my_printf("entermessage: mystical error can't make message\n");
    if (tmpuser)
      freeuser(tmpuser);
    return(FALSE);
  }

  mh = (struct mheader *)(void *)tmpstart;
  len = (mh->hlen + mh->len + 4) & ~0x03;

  /* Touch to insure memory we'll need is paged in */
  mmpos = msg->curpos;
#if 1
  mmpos = mmpos + len >= MM_FILELEN ? 0L : mmpos;
#else
  /* Msgsize should be a constant in msg-> rather than a global? */
  mmpos = mmpos + len >= msgsize ? 0L : mmpos;
#endif
  for (i = 0; i < len; i += 1024)
    foo = *((char *)msgstart + mmpos + i) + *((char *)tmpstart + i);
  foo = ouruser->usernum;
  if (tmpuser)
    foo = tmpuser->usernum;

  locks(SEM_MSG);

  /* Store the message in the main message file */
  mh->msgid = mmhi = ++msg->highest;
#if 1
  msg->curpos = (mmpos = msg->curpos + len >= MM_FILELEN ? 0L : msg->curpos) + len;
#else
  msg->curpos = (mmpos = msg->curpos + len >= msgsize ? 0L : msg->curpos) + len;
#endif
  bcopy((char *)tmpstart, (char *)msgstart + mmpos, len);

  /* Now add a pointer to this message in the fullrm file for this room */
  msg->room[curr_rm].posted++;
  fr_post(curr_rm, msg->room[curr_rm].posted, mmpos, mmhi, tmpuser);

  /* Re-post a copy if to/from Sysop or is a Yell */
  if (tosysop && mtype != MES_XYELL)
    fr_post(MAIL_RM_NBR, ++msg->room[MAIL_RM_NBR].posted, mmpos, mmhi, NULL);
  else if (mtype == MES_SYSOP || mtype == MES_FM)
  {
    fr_post(SYSOP_RM_NBR, ++msg->room[SYSOP_RM_NBR].posted, mmpos, mmhi, tmpuser);
    if (curr == AIDE_RM_NBR)
      ouruser->yells++;
  }

  unlocks(SEM_MSG);

  munmap((void *)tmpstart, 53248);

  if (tmpuser)
    freeuser(tmpuser);

  ouruser->posted++;
  if (room->num[MSGSPERRM - 1] != savedhighest)
  {
    long sh = savedhighest;

    loadroom();
    if (ouruser->lastseen[curr] == sh && !ouruser->f_ownnew && room->num[MSGSPERRM - 2] == sh)
      ouruser->lastseen[curr] = savedhighest;
  }

  if (mtype == MES_XYELL)
    my_printf("\nX messages saved in a Yell to Sysop.  Please allow up to 24 hours for the\nsysops to have time to respond.\n");
  else if (tosysop)
    my_printf("\nYour Yell has been saved.  Please allow up to 24 hours for the sysops to have\ntime to respond.\n");
  return(TRUE);
}




/*
 * Messages now have a struct containing some binary data as their header, to
 * making parsing messages much quicker and easier.  Messages are headed by a
 * special magic byte on a word (4 byte) boundary that is unique (M_MAGIC).
 */

/*
 *   Copy the POST_E POST_S stuff from readmessage()'s wrapper
 */
int
newreadmessage(
  unsigned char *p,
  int    *auth,         /* set this parameter to YES if author */
  char   *aname,
  int     new,          /* TRUE to skip new message author just wrote */
  long    msgid)        /* message id (validity check -- if 0 ignore it) */
{
struct mheader *mh;
char    title[120];	/* This used to be 70.  Caused seg faults.  Bad -JB */
char    work[70];
char    authfield[20];
int     linenbr;
int     msgsize;
int     msgpos;
char    *name;
long    mstartpos;
int     show = 1;
long    oldpos;
int i;


  mh = (struct mheader *)(void *)p;

  if (!mh)
  {
    my_printf ("readmessage:  no mh!\n");
    return (MNFERR);
  }
  if ((int)p & 3) // This is appears to be testing for an address not aligned to 32-bit boundary. But why?
  {
    printf ("wtf?\r\n");
    return(MNFERR);
  }
  if (mh->magic != M_MAGIC)
  {
    printf ("no magic\r\n");
    return(MNFERR);
  }
  if (mh->mtype < MES_NORMAL || mh->mtype > MES_FM)
  {
    my_printf("\nReadmessage: error in message type\n");
    return(FMTERR);
  }
  if (msgid > 0 && mh->msgid != msgid)
    return(MNFERR);

  if (msgid)
    sysopflags &= ~SYSOP_MSG;

  if (mh->mtype == MES_ANON)
    strcpy(title, "@Y  ***********  ");
  else if (mh->mtype == MES_AN2)
    strcpy(title, "@Y  -anonymous-  ");
  else
    *title = 0;
  strcpy(authfield, title);

  sprintf(work, "@M%s", formtime (2, mh->ptime));
  // sprintf(work, "@M%s %d, %d %02d:%02d", months[mh->month], mh->day, 1900 + mh->year, mh->hour, mh->minute);
  strcat(title, work);

  name = getusername(mh->poster, 1);
  if (mh->mtype == MES_DESC)
  {
    sprintf(work, "@G by @C%s", name);
    strcat(title, work);
// neuro edit
// don't fucking ask me how half this shit works, it appears the forum info
// gets rewritten every time somebody reads it (?!) but this works once every
// user is logged in with the right bbs binary
    if (msg->room[curr].roomaide)
    {
      name = getusername(msg->room[curr].roomaide, 0);
      if (!name)
        name = "Sysop";
    } else {
      name = "(Sysop)";
    }
// end neuro edit
    strcpy(aname, name);
    colorize("@G\nForum moderator is @C%s@G.  Total messages:@R %ld\n@GForum info last updated ", name, msg->room[curr].posted);
    strcat(title, "\n");
  }
  else
  {
    if (ouruser->usernum == mh->poster)
      if (new && !ouruser->f_ownnew)
        return (REPERR);
      else
        *auth = TRUE;
    else
      *auth = FALSE;

    if (msgid > 0)
      strcpy(aname, name);         /* send name back to caller */
    if (ouruser->usernum != mh->poster && *name != '<')
      strcpy(profile_default, name);
    if ((mh->mtype != MES_ANON && mh->mtype != MES_AN2) || curr == AIDE_RM_NBR)
    {
      if (mh->mtype == MES_SYSOP)
        sysopflags |= SYSOP_MSG;
      sprintf(work, "@G from @C%s%s%s", name, mh->mtype == MES_FM ? " (Forum Moderator)" : "", (mh->mtype == MES_SYSOP && (!mh->mail || (*auth && ouruser->f_aide || !*auth && !ouruser->f_aide))) ? " (Sysop)" : "");
      strcat(title, work);
    }
    else
      if (!*auth && !ouruser->f_prog && ouruser->usernum != msg->room[curr].roomaide)
      {
        show = 0;
        *profile_default = '\0';
      }
      else if (*auth)
      {
        sprintf(work, "@G from @C%s", name);
        strcat(title, work);
      }
  }

  if (mh->mail)
  {
    if (mh->mtype == MES_NORMAL && ouruser->usernum != mh->ext.mail.recipient && curr == MAIL_RM_NBR && !*auth)
      return(MNFERR);
    name = getusername(mh->ext.mail.recipient, 1);
    sprintf(work, "@G to @C%s%s", name, (mh->mtype == MES_SYSOP && (*auth && !ouruser->f_aide || !*auth && ouruser->f_aide)) ? " (Sysop)" : "");
    strcat(title, work);
  }
  else if (mh->quotedx)
    strcat(title, " @C(Quoted X's)");

  if (curr != MAIL_RM_NBR && mh->mtype != MES_DESC && curr != mh->forum && curr != AIDE_RM_NBR)
  {
    sprintf(work, "@G in @Y%s>", msg->room[mh->forum].name);
    strcat(title, work);
  }

  if (new)
    my_putchar('\n');

  colorize(show || mh->mtype == MES_DESC ? title : authfield);
  colorize("@G\n");

  p += mh->hlen;
  msgsize = mh->len;
  msgpos = 0;
  linenbr = mh->mtype == MES_DESC ? 3 : 1;

  for (;;)
    if (!(i = *p++) || ((++msgpos, my_putchar(i)) == '\n' && ++linenbr >= rows - 1 && line_more(&linenbr, (msgpos * 100) / msgsize) < 0))
      break;

  return(0);
}




int
readmessage(
  unsigned char *p,
  int    *auth,         /* set this parameter to YES if author */
  char   *aname,
  int     new,          /* TRUE to skip new message author just wrote */
  long    msgid)        /* message id (validity check -- if 0 ignore it) */
{
  int ret;

  if (client)
  {
    my_putchar(IAC);
    my_putchar(POST_S);
  }

  ret = newreadmessage(p, auth, aname, new, msgid);

  if (client)
  {
    my_putchar(IAC);
    my_putchar(POST_E);
    if (numposts++ < 0)
      numposts = -numposts + 2;
  }

  return(ret);
}




int
makemessage(
  struct user *recipient, /* on entry, NULL if its not mail */
  int     mtype,	/* MES_NORMAL, MES_ANON, MES_AN2, MES_DESC */
  int     upload)	/* TRUE if user wants to end a message via ctrl-D */
{
int     auth;
int     chr = CTRL_D;
int     cmd;
int     i;
int     lnlngth;	/* line length */
int     lastspace;	/* char position of last space encountered on line */
int     cancelspace;	/* true when last character on line is a space */
char    thisline[MARGIN + 1];	/* array to save current line */
time_t  now;
char    dummy[MAXALIAS + 1];
int     invalid = 0;
struct mheader *mh;
unsigned char *tmpp;
unsigned char *tmpsave;

  {
    int size = 53248;
    if (!(tmpstart = (unsigned char *)mymmap(NULL, &size, 1)))
      return(MMAP_ERR);
  }

  mh = (struct mheader *)(void *)tmpstart;

  mh->magic = M_MAGIC;
  mh->mtype = mtype;
  mh->poster = ouruser->usernum;

  time(&now);
  mh->ptime = now;

  /* Mail posts and room descriptions use this field */
  if (recipient && mtype != MES_XYELL)
  {
    mh->mail = 1;
    mh->ext.mail.recipient = recipient->usernum;
    mh->hlen = sizeof *mh - sizeof mh->ext + sizeof mh->ext.mail;
  }
  else
    mh->hlen = sizeof *mh - sizeof mh->ext;

  mh->forum = curr;

  tmpp = tmpsave = tmpstart + mh->hlen;

  if (mtype == MES_XYELL)
  {
    if ((i = xyell(recipient, tmpp)) <= 0)
    {
      my_putchar('\n');
      return(ABORT);
    }
    tmpp[i] = 0;
    mh->quotedx = 1;
    mh->len = i;
    return(SAVE);
  }

  *tmpp = 0;		/* Marks end of message for pre-entering header */

  if (recipient)
    my_putchar('\n');

  readmessage(tmpstart, &auth, dummy, FALSE, 0);

  if (client)
  {
    my_putchar(IAC);
    my_putchar(G_POST);
    my_putchar(upload);
    my_putc((byte >> 16) & 255, stdout);
    my_putc((byte >> 8) & 255, stdout);
    my_putc(byte & 255, stdout);
    block = 1;
    upload = 1;
  }

  lnlngth = 0;
  lastspace = 0;
  cancelspace = FALSE;

  for (;;)
  {
    if (tmpp - tmpstart > 50000)
    {
      if (!client)
      {
        my_printf("\nMessage too long, must Abort or Save\n\n");
        flush_input(1);
      }
      while ((chr = inkey()) != CTRL_D)
        ;
    }
    else
      chr = inkey();

    if (client)
    {
      if (lnlngth < MARGIN - 1 && chr >= SP && chr < DEL)
      {
        *tmpp++ = chr;
        lnlngth++;
        continue;
      }
      else if (chr == LF || chr == CTRL_D || chr == TAB && lnlngth >= MARGIN - 8)
      {
	for (; lnlngth && tmpp[-1] == SP; tmpp--, lnlngth--)
	  ;
	if (lnlngth || chr != CTRL_D)
	  *tmpp++ = LF;
	lnlngth = 0;
	if (chr != CTRL_D)
          continue;
      }
      else if (chr == TAB)
      {
	do
	{
	  *tmpp++ = SP;
	  lnlngth++;
	}
	while (lnlngth & 7);
        continue;
      }
      else
      {
	my_printf("\n\nClient error while posting, message not saved!  Please see Client> forum\n\n");
	while (inkey() != CTRL_D)
	  ;
	return(ABORT);
      }
    }
    else
    {
      if (chr < ' ' && (chr != CTRL_D || !upload) && chr != BS && chr != TAB && chr != LF && chr != CTRL_X && chr != CTRL_W && chr != CTRL_R)
      {
        if (invalid++)
          flush_input(invalid < 6 ? (invalid / 2) : 3);
        continue;
      }
      invalid = 0;
  
      if (chr == BS)
      {
        if (lnlngth)
        {
          my_putchar(BS);
          my_putchar(SP);
          my_putchar(BS);
          if (lnlngth-- == lastspace)
            for (; --lastspace && thisline[lastspace] != SP;)
              ;
        }
        continue;
      }
      else if (chr == CTRL_X)
      {
        for (; lnlngth; lnlngth--)
        {
          my_putchar(BS);
          my_putchar(SP);
          my_putchar(BS);
        }
        lastspace = 0;
        continue;
      }
      else if (chr == CTRL_W)
      {
        for (; lnlngth && thisline[lnlngth] == SP; lnlngth--)
        {
          my_putchar(BS);
          my_putchar(SP);
          my_putchar(BS);
        }
        for (; lnlngth && thisline[lnlngth] != SP; lnlngth--)
        {
          my_putchar(BS);
          my_putchar(SP);
          my_putchar(BS);
        }
        lastspace = lnlngth;
        continue;
      }
      else if (chr == CTRL_R)
      {
        thisline[lnlngth + 1] = 0;
        my_printf("\n%s", thisline + 1);
        continue;
      }
      else if (chr == TAB)
        if (lnlngth >= MARGIN - 8)
          chr = LF;
        else
	{
          for (thisline[++lnlngth] = my_putchar(SP); lnlngth & 7;)
            thisline[++lnlngth] = my_putchar(SP);
	  continue;
	}
      else if (chr == SP && cancelspace)
      {
	cancelspace = 0;
	continue;
      }
      else
        cancelspace = FALSE;
  
      if (chr != LF && chr != CTRL_D)
        lnlngth++;
  
      if (chr == SP && ((lastspace = lnlngth) == MARGIN))
      {
        cancelspace = TRUE;
        chr = LF;
	lnlngth--;
      }
      else if (lnlngth == MARGIN)
        if (lastspace > MARGIN / 2)
        {
          for (lnlngth--; lnlngth > lastspace; lnlngth--)
          {
            my_putchar(BS);
            my_putchar(SP);
            my_putchar(BS);
          }
          for (i = 1; i < lnlngth; i++)
            *tmpp++ = thisline[i];
          *tmpp++ = my_putchar(LF);
          for (lnlngth = 1; lnlngth + lastspace < MARGIN; lnlngth++)
          {
            i = thisline[lnlngth + lastspace];
            thisline[lnlngth] = my_putchar(i);
          }
          lastspace = 0;
        }
        else
        {
          for (i = 1; i < MARGIN; i++)
            *tmpp++ = thisline[i];
          *tmpp++ = my_putchar(LF);
          lastspace = 0;
          lnlngth = 1;
        }
  
      if (chr != CTRL_D && chr != LF)
      {
        thisline[lnlngth] = my_putchar(chr);
	continue;
      }
      else if (lnlngth || chr == LF && upload)
      {
	int save = lnlngth;

        for (; lnlngth && thisline[lnlngth] == ' '; lnlngth--)
          ;
        for (i = 1; i <= lnlngth; i++)
          *tmpp++ = thisline[i];
        *tmpp++ = my_putchar(LF);
	lastspace = lnlngth = 0;
	if (chr == LF && (save || upload))
	  continue;
      }
    }


    /* Two LFs or ctrl-D when uploading reach here */
    mh->len = tmpp - tmpsave;
    *tmpp = 0;

    for (i = 0, cmd = ' '; cmd != 'C' && cmd != 'P'; )
    {
      checkx(1);
      if (!client && cmd != 'q' && i != 1)
	colorize("@Y<A>@Cbort @Y<C>@Continue @Y<P>@Crint @Y<S>@Cave @Y<X>@Cpress ->@G ");
      cmd = get_single_quiet(" \nACPSqQx?/");
      if (strchr("qQx?/", cmd))
	i = 0;

      if (guest && (cmd == 'Q' || cmd == 'x'))
      {
	my_printf("\n\nThe Guest user cannot do that.\n\n");
	continue;
      }

      switch (cmd)
      {
	case '\n':
	case ' ':
	  if (!i++)
	    continue;
	  flush_input(i < 6 ? (i / 2) : 3);
	  my_putchar('\n');
	  break;

	case 'A':
	  if (!client)
	  {
	    colorize("@RAbort:@G are you sure? ");
	    if (yesno(-1) == NO)
	      break;
	  }
	  my_putchar('\n');
	  return (ABORT);

	case 'C':
	  if (client)
	    break;
	  my_printf("Continue...\n");
	  continue;

	case 'P':
	  my_printf("Print formatted\n\n");
	  readmessage(tmpstart, &auth, dummy, FALSE, 0);
	  continue;

	case 'S':
	  if (!client)
	    my_printf("Save message\n");
	  if (tmpp < tmpsave + 3)
	  {
	    my_printf("\nEmpty message -- ");
	    return(ABORT);
	  }
	  i = (++postcount - 10) * 30;
	  time(&now);
	  if (now - ouruser->time < i)
	    flush_input(i - now + ouruser->time);
	  return (SAVE);

	case 'Q':
	  get_syself_help(cmd);
	  my_putchar('\n');
	  break;

	case 'x':
	  express(10);
	  my_putchar('\n');
	  break;

	case '?':
	case '/':
	  my_printf("\n\nThe help for this section is not yet available.\n");
	  my_putchar('\n');
	  break;
      }				/* switch (cmd) */

      i = 0;
      if (client)
      {
	my_putchar(IAC);
	my_putchar(G_POST);
	my_putchar(upload);
	my_putc((byte >> 16) & 255, stdout);
	my_putc((byte >> 8) & 255, stdout);
	my_putc(byte & 255, stdout);
	block = 1;
      }
    }
  }
}
