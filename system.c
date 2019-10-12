/*
 * system.c - Handles checking passwords, setting up user information.
 */
#include "defs.h"
#include "ext.h"


/*
 * login_user (name, passwd, time_sys, laston); 
 *
 * Validate the user name and password against the passwd file.  If correct,
 * return a pointer to the user structure, else return NULL. This function, if
 * successful, returns a pointer to the user structure. 
 */

struct user *
login_user(const char   *name, const char   *passwd)
{
struct user *up;

  guest = !strcmp(name, "Guest");
  if (!(up = getuser(name)))
    return(NULL);

  if (guest)
  {
    struct user *tmpuser;

    tmpuser = (struct user *)mmap(0, sizeof(struct user), PROT_READ | PROT_WRITE, MAP_ANONYMOUS /* | MAP_VARIABLE */ | MAP_PRIVATE, -1, 0);
    if (!tmpuser || tmpuser == (struct user *)-1)
      return(NULL);
    bcopy(up, tmpuser, sizeof(struct user));
    return(up = tmpuser);
  }

  if (!getenv("BBSNAME"))
  {
    const char* cp = crypt(passwd, up->passwd);
    if (strncmp(up->passwd, cp, 13))
    {
      freeuser(up);
      return (NULL);
    }
  }
  return (up);
}


/*
 * change_password (what_user, old_pass, new_pass, noold) 
 *
 * Returns TRUE if old password correct 
 */

void
change_password(struct user *up, const char*old, const char* new, int noold)
{
time_t  salt;
char    saltc[4];
int     c,
        i;

  /*
   * Now create the new password 
   *
   * This uses the proceedure in the Berkeley passwd.c file 
   *
   */

   /* (void) */ time(&salt);
  salt = 9 * pid;
  saltc[0] = salt & 077;
  saltc[1] = (salt >> 6) & 077;
  for (i = 0; i < 2; i++)
  {
    c = saltc[i] + '.';
    if (c > '9')
      c += 7;
    if (c > 'Z')
      c += 6;
    saltc[i] = c;
  }
  saltc[2] = 0;
  locks(SEM_USER);
  strcpy(up->passwd, (char *)crypt(new, saltc));
  unlocks(SEM_USER);
}

int
new_user(void)
{
  int i;
  int j;
  int c;
  char *p;
  char *name;
  struct user *tmpuser = NULL;
  char pas[9];
  char pas2[14];
  char real_name[MAXNAME+1];
  char addr1[MAXNAME+1];
  char city[21];
  char state[21];
  char zip[11];
  char phone[21];
  char mail[MAXNAME+1];
  char name2[MAXALIAS+1];
  char work[80];
  int anonymous;
  int salt;
  char saltc[3];
  long usernum;

  termset();

  if (nonew == 2)
  {
    help("new.disallow", NO);
    my_exit(10);
  }

/*
  if (!yesno(-1))
  {
    my_printf("\n\nOK, you'll be put back at the regular login prompt where you may login as\n'Guest' to look around.\n\n");
    return(-1);
  }
*/

  if (nonew)
  {
    help("new.restricted", NO);
    hit_return_now();
  }

  strcpy(mybtmp->name, "<Newbie>");
  nonew = -1 - nonew;

  for (;;)
  {
  // newuser_bugfix
  int breakflag = NO;
    {
      my_printf("\nPlease choose a name, up to %d letters long: ", MAXALIAS - 1);
      flush_input(0);
      name = get_name("", 1);
      check_quit(name);
      for (i = 0, j = strlen(name); i < j; i++)
        name2[i] = tolower(name[i]);
      name2[i] = 0;
      if (!strcmp(name2, "new") || strstr(name2, "sysop") || strstr(name2, "moderator") || /*strstr(name2, "isca") || !strcmp(name2, "guest") ||*/ (tmpuser = getuser(name)))
      {
        if (tmpuser)
        {
          freeuser(tmpuser);
          tmpuser = NULL;
	  my_printf("\nThat name is already in use.");
          // newuser_bugfix
          breakflag = YES;
        }
	else {
	  my_printf("\nThat name is not allowed.");
          // newuser_bugfix
          breakflag = YES;
        }
        my_printf("\nPlease try again or hit ctrl-D to exit.\n");
      }
      else if (j == 1 || j >= MAXALIAS)
      {
        my_printf("\nThat name is too %s.  Please choose another.\n", j == 1 ? "short" : "long");
        // newuser_bugfix
        breakflag = YES;
      }
    // newuser_bugfix
    if (breakflag == YES)
      continue;
    }
  
    help("new.password", NO);
    for (;;)
    {
      flush_input(0);
      get_string("Choose a password, at least six characters long: ", -8, pas, -1);
      check_quit(pas);
      if ((j = strlen(pas)) < 6)
        my_printf("\nYour password must be at least 6 characters long.  Please try again.\n\n");
      else if (!strcasecmp(name, pas))
        my_printf("\nYou cannot use your username as your password!\n\n");
      else
      {
        flush_input(0);
        get_string("Again for verification: ", -8, pas2, -1);
        check_quit(pas2);
        if (!strcasecmp(pas, pas2))
          break;
        my_printf("\nYour passwords didn't match.  Please try again.\n\n");
      }
    }
 
    help("new.realname", NO);
    for (;;)
    {
      flush_input(0);
      get_string("\nYour full name (first AND last!): ", MAXNAME, real_name, -1);
      check_quit(real_name);
      for (i = 0, j = strlen(real_name); i < j; i++)
	if (!isalpha(real_name[i]) && real_name[i] != ' ' && real_name[i] != '-' && real_name[i] != '.' && real_name[i] != ',')
	  break;
      if (!j)
	my_printf("\nYou have to provide your name!\n");
      else if (i < j)
	my_printf("\nThe character \"%c\" is not allowed in a name.\n", real_name[i]);
      else if (j < 4 || !strchr(real_name, ' ') || real_name[j - 2] == ' ' 
              || (real_name[j - 3] == ' ' && real_name[j - 1] == '.'))
	my_printf("\nYou must provide both your full first and last names!\n");
      else if (!strcmp(real_name, name) || !strcmp(real_name, name2))
      {
	help("new.namesame", NO);
        my_printf("Are you sure you want to do this? (Y/N) -> ");
	flush_input(0);
	if (!yesno(-1))
	{
	  my_printf("\n\nOK, we'll let you select a different name for yourself...\n\n");
	  name = 0;
	}
	break;
      }
      else
	break;
    }
    if (!name)
      continue;

    for (;;)
    {
      flush_input(0);
      get_string("Street address: ", MAXNAME, addr1, -1);
      check_quit(addr1);
      if (!strlen(addr1))
        my_printf("\nYou have to provide your address!\n");
      else
        break;
    }

    for (;;)
    {
      flush_input(0);
      get_string("City: ", sizeof city, city, -1);
      check_quit(city);
      if (!strlen(city))
        my_printf("\nYou have to provide your city!\n");
      else
        break;
    }

    for (;;)
    {
      flush_input(0);
      get_string("State/country: ", 20, state, -1);
      check_quit(state);
      if (!strlen(state))
        my_printf("\nYou have to provide your state/country!\n");
      else
        break;
    }

    for (;;)
    {
      flush_input(0);
      get_string("ZIP or mail code: ", 10, zip, -1);
      check_quit(zip);
      if (!strlen(zip))
        my_printf("\nYou have to provide your ZIP or mail code!\n");
      else
        break;
    }

    get_string("Phone number (including area code): ", 20, phone, -1);
    check_quit(phone);

    for (;;)
    {
      flush_input(0);
      get_string("E-mail address: ", MAXNAME, mail, -1);
      check_quit(mail);
      if (!*mail)
        my_printf("\nYou MUST enter a valid e-mail address to be allowed to use this BBS!  If you do\nnot have one, we are sorry, you cannot be allowed to use this BBS!\n\n");
      else if (!strncmp(mail, "in%", 3) || !strncmp(mail, "IN%", 3) || !strncmp(mail, "smtp%", 5) || !strncmp(mail, "SMTP%", 5))
        my_printf("\nIf your e-mail address begins with IN%% or SMTP%%, please provide it without that\nheader and with any \"'s (quotation marks) removed.  It should be of the form\naccountname@sitename.  An example would be bbs@bbs.isca.uiowa.edu\n\n");
      else if (strchr(mail, '"'))
        my_printf("\nA valid e-mail address does not have any \"'s (quotation marks) in it.  Please\nprovide your e-mail address absent any quotation marks.\n\n");
      else if (!(p = (char *)strchr(mail, '@')) || !strchr(mail, '.') || strchr(mail, ' '))
        my_printf("\nThat is not a valid e-mail address, please try again.\n\n");
      else if (isdigit(p[1]))
	my_printf("\nYou must use a name, not a numeric address, following the '@' in your e-mail\naddress.  Please try again.\n\n");
      else if (!strcasecmp(p, "@bbs.isca.uiowa.edu") || !strcasecmp(p, "@whip.isca.uiowa.edu"))
        my_printf("\nThat is NOT your e-mail address.  Please try again.\n\n");
      else if (!strcasecmp(p, "@chop.isca.uiowa.edu") || !strcasecmp(p, "@panda.isca.uiowa.edu") || !strcasecmp(p, "@panda.uiowa.edu"))
	my_printf("\nSorry, that e-mail address is not valid for validation purposes.  You cannot\nuse an e-mail address from the Panda system.  Please try again.\n\n");
      else
      {
	c = p[5];
	p[5] = 0;
	if (!strcasecmp(p, "@anon") && c == '.')
          my_printf("\nYou may not use an anonymous site as your e-mail address!\n\n");
	else
	{
	  p[5] = c;
	  break;
	}
      }
    }

    my_printf("\n\nOK, now we've got everything we need.  You can now review what you have\nprovided and be given a chance to correct yourself if you made a mistake.\n\nYou entered:\n\nName: %s\nStreet: %s\nCity: %s\nState/Country: %s\nZIP/mail code: %s\nPhone: %s\n\nE-mail address: %s\n\n\n", real_name, addr1, city, state, zip, phone, mail);
    my_printf("Does this look correct? (Y/N) -> ");
    flush_input(0);
    if (yesno(-1))
      break;
    my_printf("\n\n\nOK, we'll start again from the beginning.\n\n");
  }

  help("new.anonymous", NO);
  my_printf("Hide your real identity? (Y/N) -> ");
  flush_input(0);
  anonymous = yesno(-1);

  my_printf("\nOK, that's it, your account is now being created...");
  fflush(stdout);

  salt = 9 * pid;
  saltc[0] = salt & 077;
  saltc[1] = (salt >> 6) & 077;
  for (i = 0; i < 2; i++)
  {
    c = saltc[i] + '.';
    if (c > '9')
      c += 7;
    if (c > 'Z')
      c += 6;
    saltc[i] = c;
  }
  saltc[2] = 0;
  strcpy(pas2, crypt(pas, saltc));

  locks(SEM_NEWBIE);
  usernum = ++msg->eternal;
  unlocks(SEM_NEWBIE);

  if (!(ouruser = adduser(name, usernum)))
  {
    errlog("Failed user creation of %s", name);
    return(0);
  }

  bzero((void *)ouruser, sizeof(struct user));

  strcpy(ouruser->name, name);
  strcpy(ouruser->passwd, pas2);
  strcpy(ouruser->real_name, real_name);
  strcpy(ouruser->addr1, addr1);
  strcpy(ouruser->city, city);
  strcpy(ouruser->state, state);
  strcpy(ouruser->zip, zip);
  strcpy(ouruser->phone, phone);
  strcpy(ouruser->mail, mail);
  ouruser->an_name = ouruser->an_addr = ouruser->an_location = ouruser->an_phone = ouruser->an_mail = anonymous;
  ouruser->timescalled = 1;
  ouruser->f_ownnew = ouruser->f_novice = ouruser->f_ansi = ouruser->f_newbie = ouruser->f_nobeep = 1;
  ouruser->f_trouble = (nonew == -4);
  ouruser->f_prog = ouruser->f_admin = usernum == 1;

  /* for (i = 5; i < MAXROOMS; i++) */
  for (i = 0; i < MAXROOMS; i++)
    ouruser->generation[i] = ouruser->forget[i] = NEWUSERFORGET;
  ouruser->time = mybtmp->time;
  ouruser->usernum = usernum;
  strcpy (ouruser->doing, "");

  msync((void *)ouruser, sizeof(struct user), MS_SYNC);

  strncpy(ouruser->remote, gethost(), sizeof (ouruser->remote) - 1);
  ouruser->firstcall = ouruser->time = mybtmp->time;
  strcpy(ouruser->loginname, ARGV[1] && ARGV[2] ? ARGV[2] : "");
  add_loggedin(ouruser);

  {
      char * logmsg = my_sprintf("NEWUSER %s%s%s",
                      ouruser->loginname,
                      *ouruser->loginname ? "@" : "",
                      ouruser->remote);
      logevent(logmsg);
      free(logmsg);
  }

  my_printf("\n\nYour account has been created.\n\n");

  if (ouruser->f_trouble)
  {
    help("new.restrictedbye", NO);
    my_exit(10);
  }

  /*
   * Should this go here?  Anything else we need from regular login?
   * Timescalled, etc.?
   */
  mybtmp->nox = 0;
  nonew = 0;
  dokey(ouruser);

  return(1);
}


void
check_quit(const char *s)
{
  if (!strcasecmp(s, "exit") || !strcasecmp(s, "quit") || !strcasecmp(s, "logout"))
  {
    my_printf("\n\nQuitting...\n");
    my_exit(3);
  }
}
