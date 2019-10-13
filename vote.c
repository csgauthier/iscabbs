/*
 *
 * Vote menu et al.
 */

#include "defs.h"
#include "ext.h"

static int makevotemsg(void);

// TODO: this function isn't used.
void
vote_unused_(void)
{
  int err;
  struct user *user;
  int cmd;
  int i;
  int j;
  int k;
  int direction = FORWARD;
  int yes;
  int no;
  int abstain;
  int pos = 0;
  int linenbr;

  my_printf ("Voting booth.  EXIT THIS NOW.  IT IS BROKEN.  PLS.  THX.\n\n");

  while (pos >= 0 && pos < MAXVOTES - 1)
  {

    /* Don't display nonexistent vote items */
    if (!(ouruser->f_admin) && voteinfo->vote[pos].inuse == 0)
    {
      pos += direction;
      continue;
    }

    /* Display the item message */
    my_printf ("\n\nItem #%d:\n\n%s\n", pos + 1, voteinfo->vote[pos].msg);

    /* And a little command prompt */
    colorize ("@Y[Slot # %d]  @CVote command -> @G", pos + 1);
    if (ouruser->f_admin)
      cmd = get_single_quiet ("ABDFNrRVSQ \n/?");
    else
      cmd = get_single_quiet("BFNRVSQ \n/?");
    switch (cmd)
    {
      case 'A':
  	my_printf ("Add\n");

	/* Find the first open slot */
	for (j = 0; j < MAXVOTES; j++)
	  if (voteinfo->vote[j].inuse == 0)
	    break;

	if (j >= MAXVOTES)
	{
	  my_printf ("All voting slots are full\n");
	  continue;
	}
	pos = j;

	err = makevotemsg();
	if (err == ABORT)
	{
	  munmap ((char *)tmpstart, sizeof (voteinfo->vote[pos].msg) - 1);
	  continue;
	}


	locks (SEM_VOTE);
	bcopy ((char *) tmpstart, voteinfo->vote[pos].msg, sizeof (voteinfo->vote[pos].msg) - 1);
	voteinfo->vote[pos].inuse = 1;	
	unlocks (SEM_VOTE);
	my_printf ("\n\n");
	continue;

      case 'D':
	my_printf ("Delete\n\nAre you sure? ");
	if (yesno(-1) == NO)
	  continue;

	locks (SEM_VOTE);
	bzero (&voteinfo->vote[pos], sizeof (struct vote));
	unlocks (SEM_VOTE);
	my_printf ("Item deleted\n\n\n");
	continue;

      case 'F':
        my_printf ("Forward\n");
	direction = FORWARD;
	pos += direction;
        continue;

      case 'B':
	my_printf ("Backward\n");
	direction = BACKWARD;
	pos += direction;
        continue;

      case 'R':
	if (ouruser->f_admin)
	{
          my_printf ("Current results\n");

	  linenbr = 2;
	  for (i = 1; i < MAXTOTALUSERS; i++)
	  {
	    if (voteinfo->vote[pos].answer[i] && (user = finduser (NULL, i, 0)))
	    {
	      my_printf ("%s: %c\n", user->name, voteinfo->vote[pos].answer[i]);
	      freeuser (user);
	      if (++linenbr >= rows - 1 && line_more (&linenbr, -1))
	        break;
	    }
	  }
          continue;
	}
	/* FALLTHROUGH */

      case 'r':
          my_printf ("Current results\n");
          yes = no = abstain = 0;
          for (i = 1; i < MAXTOTALUSERS; i++)
          {
            if (voteinfo->vote[pos].answer[i] == 'Y')
              yes++;
            else if (voteinfo->vote[pos].answer[i] == 'N')
              no++;
            else if (voteinfo->vote[pos].answer[i] == 'A')
              abstain++;
          }
          my_printf ("Yes:  %d  No:  %d  Abstain:  %d  total:  %d\n\n",
                yes, no, abstain, yes + no + abstain);
          continue;

      case 'V':
	my_printf ("Vote\n");
	if (voteinfo->vote[pos].answer[ouruser->usernum] != 0)
	{
	  my_printf ("You have already voted for this item\n\n\n");
	  continue;
	}

	my_printf ("Vote <y>es, <n>o, or <a>bstain -> ");
	k = get_single_quiet ("YNA Q\n");
	if (k == 'Y' || k == 'N' || k == 'A')
	{
	  locks (SEM_VOTE);
	  voteinfo->vote[pos].answer[ouruser->usernum] = k;
	  unlocks (SEM_VOTE);
	}	
	my_printf ("\n\nThank you.\n\n");
	continue;

      case ' ':
        my_printf ("Next\n");
        pos += direction;
        continue;

      case '?':
      case '/':
        my_printf ("\n<f>orward  <b>ackward  <r>esults  <v>ote  < > next  <q>uit\n");
        continue;

      default:
	my_printf ("Quit\n");
	return;
    }
  }
}


/* This is just a modified makemessage() */
static int
makevotemsg(void)
{
struct vote vote;
int     chr = CTRL_D;
int     cmd;
int     i;
int     lnlngth;        /* line length */
int     lastspace;      /* char position of last space encountered on line */
int     cancelspace;    /* true when last character on line is a space */
char    thisline[MARGIN + 1];   /* array to save current line */
time_t  now;
int     invalid = 0;
unsigned char *tmpp;
unsigned char *tmpsave;


  {
    int size = sizeof (vote.msg) - 1;
    if (!(tmpstart = (unsigned char *)mymmap(NULL, &size, 1)))
      return(MMAP_ERR);
  }

  tmpp = tmpsave = tmpstart;
  my_printf ("\n\nEnter an item description, up to %zu chars.  Use ^D to end.\n\n", sizeof (vote.msg) - 1);

  lnlngth = 0;
  lastspace = 0;
  cancelspace = FALSE;

  for (;;)
  {
    if (tmpp - tmpstart > sizeof (vote.msg) - 1)
    {
      my_printf("\nMessage too long, must Abort or Save\n\n");
      flush_input(1);
      while ((chr = inkey()) != CTRL_D)
        ;
    }
    else
      chr = inkey();

    if (chr < ' ' && (chr != CTRL_D) && chr != BS && chr != TAB && chr != LF && chr != CTRL_X && chr != CTRL_W && chr != CTRL_R)
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
    {
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
    }

    if (chr != CTRL_D && chr != LF)
    {
      thisline[lnlngth] = my_putchar(chr);
      continue;
    }
    else if (lnlngth || chr == LF)
    {
      for (; lnlngth && thisline[lnlngth] == ' '; lnlngth--)
        ;
      for (i = 1; i <= lnlngth; i++)
        *tmpp++ = thisline[i];
      *tmpp++ = my_putchar(LF);
      lastspace = lnlngth = 0;
      if (chr == LF)
        continue;
    }
  /* ctrl-D when uploading reach here */

  for (i = 0, cmd = ' '; cmd != 'C' && cmd != 'P'; )
  {
    checkx(1);
    if (!client && cmd != 'q' && i != 1)
      colorize("@Y<A>@Cbort @Y<C>@Continue @Y<P>@Crint @Y<S>@Cave @Y<X>@Cpress ->@G ");
    cmd = get_single_quiet(" \nACPSqQx?/");
    if (strchr("qQx?/", cmd))
      i = 0;
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
	my_printf("%s\n", tmpstart);
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
    }                         /* switch (cmd) */
    i = 0;
  }
}
} 
#if 0
  if (client)
  {
    my_putchar(IAC);
    my_putchar(G_FIVE);
    my_putchar(1);
    my_putc((byte >> 16) & 255, stdout);
    my_putc((byte >> 8) & 255, stdout);
    my_putc(byte & 255, stdout); 
    block = 1;
  }
  for (i = 0; i < 5 && (!i || *send_string[i - 1]); i++)
  {
    get_string(client ? "" : ">", 78, send_string[i], i);
    if (!strcmp(send_string[i], "ABORT"))
    {
      freeuser(p);
      colorize("@ReXpress message aborted.\n");
      return;
    }

    if (ouruser->f_beeps && !strcmp (send_string[0], "BEEPS"))
      override = 'b';		/* meta-x-auto-beep mode */
      		/* Finish out the for-loop */


    if (!strcmp(send_string[0], "PING"))
    {
      freeuser(p);
      override = 'p';
    }
  }
  if (!**send_string)
    override = 'p';

#endif
