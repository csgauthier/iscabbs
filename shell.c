/*
 * shell.c - Initializes everything necessary prior to entering the BBS.
 */
#include "defs.h"
#include "ext.h"


/*
 * get_name (prompt) 
 *
 * Display the prompt given and then accept a name in the form of "First Last".
 * Will automatically capitilize the name.  Backspace are allowed.  Will not
 * allow a blank entry.  Control-D quits the program. 
 *
 * quit_priv:  0 - make user enter something 1 - allow ctrl-d to quit program
 *             2 - allow blank entry         3 - allow digits & blank entry
 *
 * 0 & 3 allow MAXNAME characters, 1 & 2 allow MAXALIAS characters
 *
 */

char   *
get_name(const char *prompt, int quit_priv)
{
char *p;
int c;
int upflag;
int fflag;
int invalid = 0;

  for (;;)
  {
    my_printf("%s", prompt);
    if (client)
    {
      my_putchar(IAC);
      my_putchar(G_NAME);
      my_putchar(quit_priv);
      my_putc((byte >> 16) & 255, stdout);
      my_putc((byte >> 8) & 255, stdout);
      my_putc(byte & 255, stdout);
      block = 1;
    }

    upflag = fflag = 1;
    p = pbuf;
    for (;;)
    {
      c = inkey();
      if (c == NL)
	break;
      if (c == CTRL_D && quit_priv == 1)
      {
	my_putchar('\n');
	my_exit(1);
      }
      if (c == '_')
        c = ' ';
      if (c == SP && (fflag || upflag))
	continue;
      if (c == BS || c == CTRL_X || c == CTRL_W || c == CTRL_R || c == SP || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9' && quit_priv == 3))
        invalid = 0;
      else
      {
	if (invalid++)
	  flush_input(invalid < 6 ? (invalid / 2) : 3);
	continue;
      }
      if (c == CTRL_R)
      {
	*p = 0;
	my_printf("\n%s", pbuf);
	continue;
      }
      do
        if ((c == BS || c == CTRL_X || c == CTRL_W) && p > pbuf)
        {
          my_putchar(BS);
          my_putchar(SP);
          my_putchar(BS);
          --p;
          upflag = (p == pbuf || *(p - 1) == SP);
	  if (upflag && c == CTRL_W)
	    break;
          if (p == pbuf)
            fflag = 1;
        }
        else
	  if (p < &pbuf[!quit_priv || quit_priv == 3 ? MAXNAME : MAXALIAS] && c != BS && c != CTRL_X && c != CTRL_W)
	  {
	    fflag = 0;
	    if (upflag && c >= 'a')
	      c -= 32;
	    if (c >= '0')
	      upflag = 0;
	    else if (c == SP)
	      upflag = 1;
	    *p++ = c;
            if (!client)
	      my_putchar(c);
	  }
      while ((c == CTRL_X || c == CTRL_W) && p > pbuf);
    }
    *p = 0;
    if (p > pbuf || quit_priv >= 2)
      break;
    if (quit_priv)
      my_printf("\nPress CTRL-D to quit.\n");
    else
      my_printf("\nThis really isn't optional!\n");
  }
  if (!client)
    my_putchar(NL);

  if (p > pbuf && p[-1] == ' ')
    p[-1] = 0;

  return (pbuf);
}



/*
 * do_login() 
 *
 * Prompt the user for his username and password and then call login_user Set the
 * global variable ouruser 
 */

void
do_login(void)
{
int i;
int users;
char *name;
struct user *tmpuser = 0;
int wrong = 0;
int fd;
char *bbsname;
char pas[9];
char temp[128];
char myname[MAXALIAS + 1];

  /* We already checked this from the queue,
   * so our back door now is to telnet localhost
   */

  if ((fd = open (DOWNFILE, O_RDONLY)) > 0)
  {
    if (tty)
    {
      close (fd);
      more (DOWNFILE, NO);
       my_printf ("\nDOC Version 1.7, Fbrd rev. 0.3, m0n0 (aka vDOC 1.7).\n\n");
      my_exit (3);
    }
    else my_printf ("The BBS is down, but you will be let in.\n\n");
  }

  my_printf ("\nDOC Version 1.7, Fbrd rev. 0.3, m0n0 (aka vDOC 1.7).\n\n");

  if ((fd = open (MOTD, O_RDONLY)) > 0)
  {
    close (fd);
    more (MOTD, NO);
  }

  more (HELLOFILE, NO);
  bbsname = getenv ("BBSNAME");

  for (;;)
  {
    guest = 0;

    if (!bbsname)
      name = get_name("Name: ", 1);
    else
      strcpy(name = myname, bbsname);

    if (strcmp(name, "New"))
    {
      if ((tmpuser = getuser(name)))
        freeuser(tmpuser);
      if (tmpuser && (!bbsname || tty) && strcmp(name, "Guest"))
        get_string("Password: ", -8, pas, -1);

      if (!tmpuser || !(ouruser = login_user(name, pas)))
      {
	if (tmpuser)
	  my_printf("Incorrect login.\n");
	else
	  my_printf("There is no user %s on this BBS.\n", name);
	my_exit(3);
//        if (++wrong > 3 || bbsname)
//        {
//	  if (!bbsname)
//           my_printf("\n\nToo many attempts.  Goodbye.\n");
//          my_exit(3);
//        }
        flush_input(wrong);
        continue;
      }
      else
      {
        xinit(NO);

	my_printf("\nWelcome to %s, %s!\n", BBSNAME, ouruser->name);

	if (ouruser->f_deleted)
        {
	  if (ouruser->f_namechanged)
	    my_printf("\a\nThis account has been marked for deletion because of a request to change the\nusername from '%s' to '%s'.\n\nYou may login as '%s' using the same password.\n\n", ouruser->name, ouruser->reminder, ouruser->reminder);
          else
            my_printf("\a\nThis account has been marked for deletion, either through your choice or\nbecause you violated BBS rules by doing something such as providing\nobviously bogus profile info.  You will be logged off.\n\n");
          my_exit(10);
        }
	else if (ouruser->f_inactive)
        {
          my_printf("You seem to have been denied access to the message system.\n");
          // my_printf("Please contact ISCA (e-mail address bbs@bbs.isca.uiowa.edu) for more.\n");
          my_exit(10);
        }

	i = ouruser->time;
        users = add_loggedin(ouruser);

        if (!guest && ouruser->time)
        {
	  my_printf("Last on: %s ", formtime (3, i));
	  if (ouruser->timeoff >= i)
	       my_printf("until %s from %s\n", formtime (4, ouruser->timeoff), ouruser->remote);
	  else
	    my_printf("from %s\n", ouruser->remote);
        }

        strcpy(ouruser->loginname, ARGV[1] && ARGV[2] ? ARGV[2] : "");
	/* I added in code that will grab hostnames from tty connections. -JB */
	strncpy(ouruser->remote, gethost(), sizeof (ouruser->remote) - 1);

        if (ouruser->f_noclient)
	{
	  my_printf("\n\nYou have been disallowed use of the BBS client, you must login using telnet.\n\n");
	  my_exit(10);
	}

	checked_snprintf(temp,sizeof(temp), "%s %s%s%s/%d", client ? "CLIENT" : "LOGIN", ARGV[1] && ARGV[2] ? ARGV[2] : "", ARGV[1] && ARGV[2] ? "@" : "", ouruser->remote, mybtmp->remport);
	logevent(temp);

	++ouruser->timescalled;

	if (!guest)
	  my_printf("This is call %d.  There are %d users.\n", ouruser->timescalled, (users) );
        if (ouruser->f_aide)
           validate_users(0);

	/*
	 * Turn off expresses and tell the user this was done if user is
	 * configured for this 
	 */
	if (!guest && mybtmp->xstat)
	  my_printf("\nNOTE:  You have eXpress messages turned OFF as a default!\n");
        if (mybtmp->elf)
          my_printf("\nYou are marked as available to help others.\n");

	checkmail(ouruser, FALSE);

	termset();

        if (*ouruser->reminder)
        {
          my_printf("\n\aREMINDER:\n%s\n\n", ouruser->reminder);
          my_printf("Please hit 'Y' to acknowledge having seen this reminder -> ");
          get_single_quiet("yY");
        }

	if (ouruser->f_badinfo || ouruser->f_duplicate)
	{
	  help("badinfo", NO);
	  sleep(300);
	}

	if (guest)
	{
	  help("guestwelcome", NO);
	  hit_return_now();
	}
	return;
      }
    }
    else
      if (!(i = new_user()))
      {
	my_printf("\n\nSorry, there was some problem setting up your BBS account.\n\nPlease try again later.\n\n");
        my_exit(10);
      }
      else if (i > 0)
        return;
  }
}


void
profile_user(int all)
{
char *name;
int how = PROF_REG;

  my_printf("Profile user\n\nUser to profile? %s", all ? "[FULL PROFILE] " : "");
  if (*profile_default)
    my_printf("(%s) -> ", profile_default);
  else
    my_printf("-> ");

  name = get_name("", 2);
  if (!*name)
  {
    if (*profile_default)
      name = profile_default;
    else
      strcpy (name, ouruser->name);
  }

  if (all)
  {
    if (ouruser->f_admin)
      how = PROF_ALL;
    else if (!strcmp(name, ouruser->name))
      how = PROF_SELF;
    else
      how = PROF_EXTRA;
  }

  if (profile(name, NULL, how) < 0)
    my_printf("There is no user %s on this BBS.\n", name);
}
