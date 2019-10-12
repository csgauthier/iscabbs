/*
 * setup.c:  Handles function of the change (setup) menu.
 */
#include "defs.h"
#include "ext.h"


/*
 * Setup Menu. 
 */

void
change_setup(struct user *workuser)
{
int c = -1;
int i;
int     chflag;
struct user *up = NULL;
char *name;

  if (ouruser->f_twit)
  {
    my_printf("\nYou are not allowed to enter the change menu.\n");
    return;
  }
  if (!workuser)
  {
    workuser = ouruser;
    chflag = 0;
    if (!ouruser->f_admin)
      my_printf("\nPress '?' for a list of commands.\n");
  }
  else
    chflag = -1;


  for(;;)
  {
    if (c)
    {
      if (chflag)
        colorize("\n@YChange config (@R%s@Y) -> @G", workuser->name);
      else
        colorize("\n@YChange config -> @G");
    }

    if (ouruser->f_admin)
      c = get_single_quiet(" aACfFHIKMnNOPqQRSTUvVXZ?\n");
    else
    {
      c = get_single_quiet(" ACfHIMOPqQRSTXZ?\n");
      if (c == 'A')
	c = 'a';
    }

    if (strchr("dn", c))
    {
      c = 0;
      continue;
    }
    else if (chflag && (c == 'C' || c == 'R' || c == 'T') && !workuser->f_admin)
    {
      my_printf("\n\nMust be working on yourself to use this option.\n");
      continue;
    }
    else if (!chflag && (c == 'F' || c == 'N'))
    {
       my_printf("\n\nMust be working on a user to use this option.\n");
       continue;
    }
    else if (chflag < 0 && (c == 'U' || c == 'N'))
    {
      my_printf("\n\nCannot use this option while validating.\n");
      continue;
    }
    else if (c == 'C' && !client)
    {
      my_printf("\n\nYou are not using the BBS client.  For more information on the BBS client,\nplease see the 'Client' helpfile.\n");
      continue;
    }


    switch (c)
    {
      case 'a':
	my_printf("Change address\n");
	change_addr(workuser, chflag);
	break;

      case 'A':
	my_printf("Change sysop info\n");
  	change_aide_info(workuser);
	break;

      case 'C':
	my_printf("Client configuration\n");
        my_putchar(IAC);
        my_putchar(CONFIG);
        my_putchar(0);
        my_putc((byte >> 16) & 255, stdout);
        my_putc((byte >> 8) & 255, stdout);
        my_putc(byte & 255, stdout);
        block = 1;
	(void)get_single_quiet("\n");
	break;

      case 'f':
        my_printf ("Edit vanityflag\n");
        change_vanityflag (workuser);
        break;

      case 'F':
	my_printf("Edit flags\n");
	foptions(workuser);
	break;

      case 'H':
	my_printf("Help\n");
	help("setup", NO);
	break;

      case '?':
	my_printf("Short help\n");
        if(ouruser->f_admin) {
          help("setupshort.aide", NO);
        } else {
          help("setupshort.user", NO);
        }
	break;

      case 'I':
	my_printf("Change profile info\n\n");
	change_info(workuser);
	break;

      case 'K':
	knrooms (workuser);
	break;

      case '\n':
      case ' ':
      case 'M':
	if (chflag >= 0)
	{
	  my_printf("Return to message system\n");
	  freeuser(workuser);
	}
	else
	  my_putchar('\n');
	return;

      case 'N':
	my_printf("Change user name\n");
	change_name(workuser);
	break;

      case 'O':
	my_printf("Other options\n");
        ooptions(workuser);
        break;

      case 'P':
	my_printf("Change password\n");
	change_pass(workuser, chflag);
	break;

      case 'q':
      case 'Q':
        my_printf("\n\nIf you have a question related to this BBS to ask a Guide, you need to exit the\nconfig menu (the menu you are currently in) first.  To do so, press the space\nbar.  You may then hit shift 'Q' (capital 'Q' not lowercase 'q') to ask your\nquestion.\n");
        break;

      case 'R':
	my_printf("Change reminder\n");
	change_reminder(workuser);
        break;

      case 'S':
	my_printf("Change secret status\n");
	change_anonymous(workuser, chflag);
	break;

      case 'T':
        my_printf ("Termset\n");
        askansi();
        break;

      case 'U':
	my_printf("Change user\n");
	if ((workuser = change_user()))
	  chflag = 1;
	else
	{
	  workuser = ouruser;
	  chflag = 0;
	}
	break;

      case 'V':
	my_printf("Verify address information\n");
	do_verify(workuser, 1);
	break;

      case 'v':
	my_printf("Show verified information\n");
	my_printf("\nVerified information for user: %s\n", workuser->name);
	show_verified(workuser);
	break;

      case 'X':
	my_printf("X message configuration\n");

	while (c != '\n' && c != 'Q' && c != ' ')
	{
	  colorize("\n@Y<0-9> @GAssign QuickX  @Y<L>@Gist QuickX  @Y<O>@Gptions  @Y<U>@Gser  @Y<Q>@Guit -> ");
	  c = get_single_quiet("0123456789LUO\n Q");
	  switch (c)
	  {
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
	      my_printf ("%c\n\n", c);
	      assignquickx (c - '0', workuser);
	      break;

	    case 'L':
	      my_printf ("List QuickX\n\n");
	      my_printf ("  Key   User\n");

	      for (i = 0; i < 10; i++)
	      {
	        up = NULL;
	        if (workuser->quickx[i])
	        {
		  up = finduser (NULL, workuser->quickx[i], 0);
		  if (!up)
		  {
		    locks (SEM_USER);
		    ouruser->quickx[i] = 0L;
		    unlocks (SEM_USER);
		  }
	        }
	        my_printf ("   %d   %s\n", i, up ? up->name : "<empty>");
	      }
	      break;

	    case 'U':
	      userlist_config (workuser, chflag);
	      break;

	    case 'O':
	      my_printf ("\n\n");
	      xoptions (workuser);
	      break;

	    default:
	      my_putchar ('\n');
	      break;
	  }		/* switch */
	}		/* for (;;) */
        break;

	case 'Z':
	  my_printf ("BBS wide room zapper\n\n");
	  do_bigzap (workuser);
	  break;

    }
  }
}



void
change_addr(struct user *tuser, int chflag)
{
  char answer[41];

  my_printf("\nEnter your address information.  To leave a entry alone, just hit return.\n\n");

  if (chflag)
  {
    char * name = my_sprintf("Name [%s]: ", tuser->real_name);
    get_string(name, 40, answer, -1);
    if (*answer)
    {
      locks(SEM_USER);
      strcpy(tuser->real_name, answer);
      unlocks(SEM_USER);
    }
    free(name);
  }
  else
  {
    my_printf("Are you sure? (Y/N) -> ");
    if (!yesno(-1))
      return;
  }

  // addr1
  char * addr1 = my_sprintf("Street & house/apt# [%s]: ", tuser->addr1);
  get_string(addr1, 40, answer, -1);
  if (*answer)
  {
    locks(SEM_USER);
    strcpy(tuser->addr1, answer);
    unlocks(SEM_USER);
  }
  free(addr1);

  // addr2
  char * addr2 = my_sprintf ("                    [%s]: ", tuser->addr2);
  get_string (addr2, 40, answer, -1);
  if (*answer)
  {
    if (!strcmp (answer, "NONE"))
      *answer = 0;
    locks (SEM_USER);
    strcpy (tuser->addr2, answer);
    unlocks (SEM_USER);
  }
  free(addr2);

  // city
  char * city = my_sprintf("City [%s]: ", tuser->city);
  get_string(city, 20, answer, -1);
  if (*answer)
  {
    locks(SEM_USER);
    strcpy(tuser->city, answer);
    unlocks(SEM_USER);
  }
  free(city);

  char * state = my_sprintf("State or country [%s]: ", tuser->state);
  get_string(state, 20, answer, -1);
  if (*answer)
  {
    locks(SEM_USER);
    strcpy(tuser->state, answer);
    unlocks(SEM_USER);
  }
  free(state);

  char * zip = my_sprintf("ZIP or mail code [%s]: ", tuser->zip);
  get_string(zip, 10, answer, -1);
  if (*answer)
  {
    locks(SEM_USER);
    strcpy(tuser->zip, answer);
    unlocks(SEM_USER);
  }
  free(zip);

  char * phone = my_sprintf("Phone number (including all prefixes!) [%s]: ", tuser->phone);
  get_string(phone, 20, answer, -1);
  if (*answer)
  {
    if (!strcmp(answer, "NONE"))
      *answer = 0;
    locks(SEM_USER);
    strcpy(tuser->phone, answer);
    unlocks(SEM_USER);
  }
  free(phone);

  char * email = my_sprintf("Internet e-mail address [%s]: ", tuser->mail);
  get_string(email, 40, answer, -1);
  if (*answer)
  {
    locks(SEM_USER);
    strcpy(tuser->mail, answer);
    unlocks(SEM_USER);
  }
  free(email);

  char * www = my_sprintf("WWW address [%s]: ", tuser->www);
  get_string(www, 59, answer, -1);
  if (*answer)
  {
    if (!strcmp(answer, "NONE"))
      *answer = 0;
    locks(SEM_USER);
    strcpy(tuser->www, answer);
    unlocks(SEM_USER);
  }
  free(www);
}


void
change_aide_info(struct user *tuser)
{
  char junk[80];

  if (*tuser->aideinfo)
    my_printf("\nPrevious line of sysop info for %s:\n%s\n\nDo you wish to change this? (Y/N) -> ", tuser->name, tuser->aideinfo);
  else
    my_printf("\nDo you wish to add a line of sysop info for %s? (Y/N) -> ", tuser->name);
  if (yesno(-1))
  {
    my_printf("\nEnter a single line of sysop info for %s:\n", tuser->name);
    get_string(">", 77, junk, -1);
    locks(SEM_USER);
    strcpy(tuser->aideinfo, junk);
    unlocks(SEM_USER);
  }
}


void
change_anonymous(struct user *tuser, int chflag)
{
  int c;

  my_printf("\nYou have the option of hiding some or all of your personal information (name,\naddress, phone, and e-mail) from others on this BBS.\n\n");
  if (chflag)
  {
    my_printf("<H>ide all  <Q>uit -> ");
    c = get_single_quiet("HQ \n");
  }
  else
  {
    my_printf("<H>ide all  <U>nhide all  <S>elect individual items  <Q>uit -> ");
    c = get_single_quiet("HUSQ \n");
  }

  switch (c)
  {
    case ' ':
    case '\n':
      my_printf("Nothing changed\n");
      return;

    case 'H':
      my_printf("Hide\n\nAll information hidden.\n");
      locks(SEM_USER);
      tuser->an_name = tuser->an_addr = tuser->an_location = tuser->an_phone = tuser->an_mail = tuser->an_www = tuser->an_site = 1;
      tuser->an_all = 0;
      unlocks(SEM_USER);
      break;

    case 'U':
      my_printf("Unhide\n\nAll information public.\n");
      locks(SEM_USER);
      tuser->an_name = tuser->an_addr = tuser->an_location = tuser->an_phone = tuser->an_mail = tuser->an_www = tuser->an_site = 0;
      tuser->an_all = 0;
      unlocks(SEM_USER);
      break;

    case 'S':
      my_printf("Select\n\nSelect individual items to hide...\n\n");

      if (tuser->an_all)
      {
        locks(SEM_USER);
        tuser->an_name = tuser->an_addr = tuser->an_location = tuser->an_phone = tuser->an_mail = tuser->an_www = tuser->an_site = 1;
        tuser->an_all = 0;
        unlocks(SEM_USER);
      }

      my_printf("Do you want to hide your real name? -> ");
      if (yesno(tuser->an_name) != tuser->an_name)
      {
	locks(SEM_USER);
	tuser->an_name ^= 1;
	unlocks(SEM_USER);
      }

      my_printf("Do you want to hide your address? -> ");
      if (yesno(tuser->an_addr) != tuser->an_addr)
      {
	locks(SEM_USER);
	tuser->an_addr ^= 1;
	unlocks(SEM_USER);
      }

      my_printf("Do you want to hide your city/state/zip? -> ");
      if (yesno(tuser->an_location) != tuser->an_location)
      {
	locks(SEM_USER);
	tuser->an_location ^= 1;
	unlocks(SEM_USER);
      }

      my_printf("Do you want to hide your phone number? -> ");
      if (yesno(tuser->an_phone) != tuser->an_phone)
      {
	locks(SEM_USER);
	tuser->an_phone ^= 1;
	unlocks(SEM_USER);
      }

      my_printf("Do you want to hide your e-mail address? -> ");
      if (yesno(tuser->an_mail) != tuser->an_mail)
      {
	locks(SEM_USER);
	tuser->an_mail ^= 1;
	unlocks(SEM_USER);
      }

      my_printf("Do you want to hide your WWW address? -> ");
      if (yesno(tuser->an_www) != tuser->an_www)
      {
	locks(SEM_USER);
	tuser->an_www ^= 1;
	unlocks(SEM_USER);
      }

      break;
  }
}

void
change_pass(struct user *tuser, int noold)
{

char    pas[9],
        original[9], pas2[9];
char temp[MAXALIAS + 1];
int i;
char *cp;

  my_printf("\nChanging password for %s...\n", tuser->name);
  if (!noold)
  {
    get_string("Old Password: ", -8, original, -1);
    if (!*original)
    {
      my_printf("Ok, so I won't change it then.\n");
      return;
    }
    cp = (char *)crypt(original, tuser->passwd);
    if (strncmp(tuser->passwd, cp, 13))
    {
      my_printf("Incorrect old password!\n");
      return;
    }
  }
  get_string("New Password: ", -8, pas, -1);
  get_string("Again for verification: ", -8, pas2, -1);
  if (strcmp(pas, pas2))
  {				/* If they didn't match */
    colorize("\n@RYour passwords didn't match.  Please try again.\n\n");
    return;
  }
  if (strlen(pas) < 6)
  {
    colorize("\n@RYour password must be at least 6 characters long.  Please try again.\n\n");
    return;
  }
  for (i = 0; i <= strlen(tuser->name); i++)
    if (tuser->name[i] >= 'A' && tuser->name[i] <= 'Z')
      temp[i] = tuser->name[i] + 32;
    else
      temp[i] = tuser->name[i];
  for (i = 0; i <= strlen(pas2); i++)
    if (pas2[i] >= 'A' && pas2[i] <= 'Z')
      pas2[i] += 32;
  if (!strcmp(temp, pas2))
  {
    my_printf("\nYou cannot use your name as your password!\n\n");
    return;
  }

  change_password(tuser, original, pas, noold);
  my_printf("\nSo be it.\n");
}


void
change_reminder(struct user *tuser)
{
  char junk[80];

  if (*tuser->reminder)
    my_printf("\nCurrent Reminder line is:\n\n%s\n\nDo you want to change this? (Y/N) -> ", tuser->reminder);
  else
    my_printf("\nDo you want to set a reminder? (Y/N) -> ");
  if (!yesno(-1))
    return;
  my_printf("\nEnter a single line of what you want yourself reminded of upon login:\n(Hit return to leave it blank and turn off the reminder)\n");
  get_string(">", 77, junk, -1);
  locks(SEM_USER);
  strcpy(tuser->reminder, junk);
  unlocks(SEM_USER);
}


/*
 * Change user info 
 *
 * Allow the user to change the description part of his/her profile. 
 */

void
change_info(struct user *tuser)
{
  char junk[5][80];
  int i;

  if (*tuser->desc1)
    my_printf("\nYour current info:\n %s\n", tuser->desc1);
  if (*tuser->desc2)
    my_printf(" %s\n", tuser->desc2);
  if (*tuser->desc3)
    my_printf(" %s\n", tuser->desc3);
  if (*tuser->desc4)
    my_printf(" %s\n", tuser->desc4);
  if (*tuser->desc5)
    my_printf(" %s\n", tuser->desc5);

  if (*tuser->desc1)
  {
    my_printf("\nDo you wish to change this? (Y/N) -> ");
    if (!yesno(-1))
      return;
    my_printf("Ok, you have five lines to do something creative.\n\n");
  }
  else
    my_printf("Enter a description, up to 5 lines\n\n");

  if (client)
  {
    my_putchar(IAC);
    my_putchar(G_FIVE);
    my_putchar(0);
    my_putc((byte >> 16) & 255, stdout);
    my_putc((byte >> 8) & 255, stdout);
    my_putc(byte & 255, stdout);
    block = 1;
  }

  *junk[1] = *junk[2] = *junk[3] = *junk[4] = 0;
  for (i = 0; i < 5 && (!i || *junk[i - 1]); i++)
    get_string(client ? "" : ">", 78, junk[i], i);

  locks(SEM_USER);
  strcpy(tuser->desc1, junk[0]); 
  strcpy(tuser->desc2, junk[1]);
  strcpy(tuser->desc3, junk[2]);
  strcpy(tuser->desc4, junk[3]);
  strcpy(tuser->desc5, junk[4]);
  unlocks(SEM_USER);
}



void
change_name(struct user *workuser)
{
  char work[60];
  struct user *tmpuser = NULL;
  char *p;
  int c;
  int i;
  int j;

  my_printf("\nNew name for user '%s' -> ", workuser->name);
  char * name = get_name("", 2);
  if (!*name)
    return;
  else if (!strcmp(workuser->name, name))
    my_printf("\nName has to be different!\n");
  else if (strlen(name) == 1)
    my_printf("\nName too short.\n");
  else if (strlen(name) >= MAXALIAS)
    my_printf("\nName too long.\n");
  else if (!strcmp(name, "new") || strstr(name, "sysop") || strstr(name, "moderator") || strstr(name, "isca") || !strcmp(name, "guest") || (tmpuser = getuser(name)))
  {
    if (tmpuser != workuser)
      freeuser(tmpuser);
    my_printf("\nName already in use.\n");
 }
  else
  {
    my_printf("\nChange name of '%s' to '%s'? (Y/N) -> ", workuser->name, name);
    flush_input(0);
    if (yesno(-1))
    {
      long usernum;

      locks(SEM_NEWBIE);
      usernum = ++msg->eternal;
      unlocks(SEM_NEWBIE);

      if (!(tmpuser = adduser(name, usernum)))
      {
        errlog("Failed sysop user creation of '%s'", name);
        my_printf("\nError creating new user file.\n");
        return;
      }

      logout_user(workuser, NULL, 0);

      bzero((void *)tmpuser, sizeof(struct user));

      tmpuser->an_all = workuser->an_all;
      tmpuser->an_name = workuser->an_name;
      tmpuser->an_addr = workuser->an_addr;
      tmpuser->an_location = workuser->an_location;
      tmpuser->an_phone = workuser->an_phone;
      tmpuser->an_mail = workuser->an_mail;
      tmpuser->an_site = workuser->an_site;
      strcpy(tmpuser->name, name);
      strcpy(tmpuser->passwd, workuser->passwd);
      strcpy(tmpuser->real_name, workuser->real_name);
      strcpy(tmpuser->addr1, workuser->addr1);
      strcpy(tmpuser->city, workuser->city);
      strcpy(tmpuser->state, workuser->state);
      strcpy(tmpuser->zip, workuser->zip);
      strcpy(tmpuser->phone, workuser->phone);
      strcpy(tmpuser->mail, workuser->mail);
      strcpy(tmpuser->aideinfo, workuser->aideinfo);
      strcpy(tmpuser->A_real_name, workuser->A_real_name);
      strcpy(tmpuser->A_addr1, workuser->A_addr1);
      strcpy(tmpuser->A_city, workuser->A_city);
      strcpy(tmpuser->A_state, workuser->A_state);
      strcpy(tmpuser->A_zip, workuser->A_zip);
      strcpy(tmpuser->A_phone, workuser->A_phone);
      strcpy(tmpuser->A_mail, workuser->A_mail);
      for (i = 5; i < MAXROOMS; i++)
        tmpuser->generation[i] = tmpuser->forget[i] = NEWUSERFORGET;
      tmpuser->f_duplicate = workuser->f_duplicate;
      tmpuser->f_admin = workuser->f_admin;
      tmpuser->f_restricted = workuser->f_restricted;
      tmpuser->f_prog = workuser->f_prog;
      tmpuser->f_badinfo = workuser->f_badinfo;
      tmpuser->f_newbie = workuser->f_newbie;
      tmpuser->f_inactive = workuser->f_inactive;
      tmpuser->f_deleted = workuser->f_deleted;
      tmpuser->f_noanon = workuser->f_noanon;
      tmpuser->f_noclient = workuser->f_noclient;
      tmpuser->f_trouble = workuser->f_trouble;
      tmpuser->f_invisible = workuser->f_invisible;
      tmpuser->f_elf = workuser->f_elf;
      tmpuser->f_twit = workuser->f_twit;
      tmpuser->f_aide = workuser->f_aide;
      tmpuser->time = time(0);
      strcpy(tmpuser->remote, "(Username changed by sysop)");
      tmpuser->usernum = usernum;

      msync((void *)tmpuser, sizeof(struct user), MS_SYNC);

      checked_snprintf(work,sizeof(work), "NAMECHANGE: %s to %s", workuser->name, name);
      logevent(work);

      locks(SEM_USER);
      strcpy(workuser->reminder, tmpuser->name);
      workuser->f_deleted = workuser->f_invisible = workuser->f_namechanged = 1;
      unlocks(SEM_USER);

      freeuser(workuser);
      workuser = tmpuser;

      my_printf("\nUsername changed.\n");

      strcpy(profile_default, name);
      profile(NULL, workuser, PROF_ALL);
    }
  }
}



void
do_verify (struct user *workuser, int ask)
{
  if (ask)
    my_printf("\nVerify information for user: %s.\nAre you sure? (Y/N) -> ", workuser->name);
  if (!ask || yesno(-1))
  {
    locks(SEM_USER);
    strcpy(workuser->A_real_name, workuser->real_name);
    strcpy(workuser->A_addr1, workuser->addr1);
    strcpy(workuser->A_city, workuser->city);
    strcpy(workuser->A_state, workuser->state);
    strcpy(workuser->A_zip, workuser->zip);
    strcpy(workuser->A_phone, workuser->phone);
    if (ask)
      strcpy(workuser->A_mail, workuser->mail);
    if (workuser->f_trouble)
    {
      workuser->f_trouble = 0;
      unlocks(SEM_USER);
    }
    else
      unlocks(SEM_USER);
  }
}


void
show_verified(struct user *workuser)
{
  my_printf("\nReal name: %s\nAddress: %s\nCity: %s\nState: %s\nZIP: %s\nPhone: %s\nEmail: %s\n\n", workuser->A_real_name, workuser->A_addr1, workuser->A_city, workuser->A_state, workuser->A_zip, workuser->A_phone, workuser->A_mail);
}



void
ooptions(struct user *tuser)
{
  int answer;


  my_printf("\nMark yourself as a novice user? -> ");
  if (yesno (tuser->f_novice) != tuser->f_novice)
  {
    locks (SEM_USER);
    tuser->f_novice ^= 1;
    unlocks (SEM_USER);
  }  

  my_printf("Show own posts when reading new? -> ");
  if (yesno(tuser->f_ownnew) != tuser->f_ownnew)
  {
    locks(SEM_USER);
    tuser->f_ownnew ^= 1;
    unlocks(SEM_USER);
  }

  my_printf("Print last old message on New message request? -> ");
  if (yesno(tuser->f_lastold) != tuser->f_lastold)
  {
    locks (SEM_USER);
    tuser->f_lastold ^= 1;
    unlocks (SEM_USER);
  }

  my_printf("Make new messages old after any read? -> ");
  if (yesno(tuser->f_clear) != tuser->f_clear)
  {
    locks (SEM_USER);
    tuser->f_clear ^= 1;
    unlocks (SEM_USER);
  }

  my_printf("Be asked if you are on an ANSI-compatible terminal when you login? -> ");
  if (yesno(tuser->f_ansi) != tuser->f_ansi)
  {
    locks(SEM_USER);
    tuser->f_ansi ^= 1;
    unlocks(SEM_USER);
  }

  my_printf("Show time in 12-hour format? -> ");
  if (yesno (tuser->f_ampm) != tuser->f_ampm)
  {
    locks (SEM_USER);
    tuser->f_ampm ^= 1;
    unlocks(SEM_USER);
  }

  my_printf("Show short version of the wholist by default? -> ");
  if (yesno (tuser->f_shortwho) != tuser->f_shortwho)
  {
    locks (SEM_USER);
    tuser->f_shortwho ^= 1;
    unlocks (SEM_USER);
  }

  my_printf("Show wholist with oldest logins first? -> ");
  if (yesno (tuser->f_revwho) != tuser->f_revwho)
  {
    locks (SEM_USER);
    tuser->f_revwho ^= 1;
    unlocks (SEM_USER);
  }



}


void
foptions(struct user *tuser)
{
  int answer;

  my_printf("\nMark user as having bad address information? -> ");
  if (yesno(tuser->f_badinfo) != tuser->f_badinfo)
  {
    locks(SEM_USER);
    tuser->f_restricted = (tuser->f_badinfo ^= 1) | tuser->f_newbie | tuser->f_duplicate;
    unlocks(SEM_USER);
    if (!tuser->f_badinfo)
      do_verify(tuser, 1);
  }

  my_printf("Mark user for deletion? -> ");
  if (yesno(tuser->f_deleted) != tuser->f_deleted)
  {
    locks(SEM_USER);
    tuser->f_invisible = (tuser->f_deleted ^= 1) | tuser->f_inactive;
    unlocks(SEM_USER);
    if (tuser->f_deleted)
      logout_user(tuser, NULL, 0);
  }

  my_printf("Twitify user? -> ");
  if (yesno(tuser->f_twit) != tuser->f_twit)
  {
    locks(SEM_USER);
    tuser->f_twit ^= 1;
    unlocks(SEM_USER);
  }

  my_printf("Mark user as a new user? -> ");
  if (yesno(tuser->f_newbie) != tuser->f_newbie)
  {
    locks(SEM_USER);
    tuser->f_restricted = (tuser->f_newbie ^= 1) | tuser->f_badinfo | tuser->f_duplicate;
    unlocks(SEM_USER);
  }

  my_printf("Make user a guide? -> ");
  if ((answer = yesno(tuser->f_elf)) && !tuser->f_elf && (tuser->timescalled < 100 || tuser->posted < 100))
  {
    my_printf("  %s has %d calls, %d posts.  Are you sure? (Y/N) -> ", tuser->name, tuser->timescalled, tuser->posted);
    answer = yesno(-1);
  }
  if (answer != tuser->f_elf)
  {
    locks(SEM_USER);
    tuser->f_elf ^= 1;
    unlocks(SEM_USER);
  }

  my_printf("Mark user as having multiple accounts? -> ");
  if (yesno(tuser->f_duplicate) != tuser->f_duplicate)
  {
    locks(SEM_USER);
    tuser->f_restricted = (tuser->f_duplicate ^= 1) | tuser->f_newbie | tuser->f_badinfo;
    unlocks(SEM_USER);
  }

  my_printf("Mark user inactive? -> ");
  if (yesno(tuser->f_inactive) != tuser->f_inactive)
  {
    locks(SEM_USER);
    tuser->f_invisible = (tuser->f_inactive ^= 1) | tuser->f_deleted;
    unlocks(SEM_USER);
  }
  if (tuser->f_inactive)
    logout_user(tuser, NULL, 0);

  my_printf("Disallow user from using a BBS client? -> ");
  if (yesno(tuser->f_noclient) != tuser->f_noclient)
  {
    locks(SEM_USER);
    tuser->f_noclient ^= 1;
    unlocks(SEM_USER);
  }

  my_printf("Disallow user from using the anon posting option? -> ");
  if (yesno(tuser->f_noanon) != tuser->f_noanon)
  {
    locks(SEM_USER);
    tuser->f_noanon ^= 1;
    unlocks(SEM_USER);
  }

/*
  if (ouruser->usernum == 2L)
  {
    my_printf ("Completely Phuck User Up? -> ");
    if (yesno(tuser->f_phucked) != tuser->f_phucked)
    {
      locks (SEM_USER);
      tuser->f_phucked ^= 1;
      unlocks (SEM_USER);
    }
  }
*/


  if (ouruser->f_admin)
  {
    my_printf ("Allow user to use the BEEPS keyword? -> ");
    if (yesno (tuser->f_beeps) != tuser->f_beeps)
    {
      locks (SEM_USER);
      tuser->f_beeps ^= 1;
      unlocks (SEM_USER);
    }

    my_printf("Make user a sysop? -> ");
    if (yesno(tuser->f_aide) != tuser->f_aide)
    {
      locks(SEM_USER);
      tuser->f_admin = (tuser->f_aide ^= 1) | tuser->f_prog;
      unlocks(SEM_USER);
    }

    my_printf("Make user a programmer? -> ");
    if (yesno(tuser->f_prog) != tuser->f_prog)
    {
      locks(SEM_USER);
      tuser->f_admin = (tuser->f_prog ^= 1) | tuser->f_aide;
      unlocks(SEM_USER);
    }
  }
}


void
xoptions(struct user *tuser)
{
  my_printf("Options\n\nHave eXpress messages turned OFF when you first enter the BBS? -> ");
  if (yesno(tuser->f_xoff) != tuser->f_xoff)
  {
    locks(SEM_USER);
    tuser->f_xoff ^= 1;
    unlocks(SEM_USER);
  }

  my_printf("Have eXpress messages arrive immediately while posting? -> ");
  if (yesno(tuser->f_xmsg) != tuser->f_xmsg)
  {
    locks(SEM_USER);
    tuser->f_xmsg ^= 1;
    unlocks(SEM_USER);
  }

  if (tuser->f_elf)
  {
    my_printf("Be 'available to help others' by default when you login? -> ");
    if (yesno(tuser->f_autoelf) != tuser->f_autoelf)
    {
      locks(SEM_USER);
      tuser->f_autoelf ^= 1;
      unlocks(SEM_USER);
    }
  }

  my_printf ("Reset your x-log to zero? -> ");
  if (yesno (0) != 0)
  {
    if (XPENDING)
      checkx (1);
    xinit (YES);    
  }
}



/*
 * Handles configuration of userlist for X message refusal/acceptance.
 */
void
userlist_config(struct user *tmpuser, int chflag)
{
  int c;
  int i, j;
  char *name, *tmpname;
  struct user *up = NULL;
  int which;

  clean_xconf(tmpuser);
  if (chflag && tmpuser->xconftime)
    my_printf("User\n\nLast modified %s", ctime(&tmpuser->xconftime));
  else
    my_printf("User\n");

  for (;;)
  {
    if (up)
    {
      freeuser(up);
      up = NULL;
    }

#if 0
    if (chflag)
#else
    if (0)
#endif
    {
      my_printf("\n<L>ist <Q>uit -> ");
      c = get_single_quiet("LQ\n ");
    }
    else
    {
      my_printf("\n<A>dd <D>elete <L>ist <Q>uit -> ");
      c = get_single_quiet("ADLQ\n ");
    }


    if (c == 'L')
    {
      my_printf("List\n\nX message enable/disable list:\n");
      if (!tmpuser->xconf[0].usernum)
	my_printf("\n(empty)\n");
      else
      {
        for (i = 0; i < NXCONF; i++)
	  if (tmpuser->xconf[i].usernum && (name = getusername(tmpuser->xconf[i].usernum, 0)) && !tmpuser->f_invisible)
	    my_printf("%s%c %-19s", !(i % 3) ? "\n" : "    ", tmpuser->xconf[i].which ? '+' : '-', name);
        my_putchar('\n');
      }
    }


    else if (c == 'A' || c == 'D')
    {
      my_printf("%s\n\nUser to %s enable/disable list -> ", c == 'A' ? "Add" : "Delete", c == 'A' ? "Add to" : "Delete from");
      name = get_name("", 2);
      if (!*name)
	continue;
      if (!(up = getuser(name)) || (up->f_invisible && c == 'A'))
      {
        my_printf("\nThere is no user %s on this BBS.\n", name);
        continue;
      }

      if (c == 'A')
      {
	my_printf("\n<E>nable or <D>isable %s -> ", name);
	which = get_single_quiet("ED");
	my_printf("%s\n", which == 'E' ? "Enable" : "Disable");

	locks(SEM_USER);

	if (tmpuser->xconf[NXCONF - 1].usernum)
	{
	  unlocks(SEM_USER);
	  my_printf("\nSorry, list is full.\n");
	  continue;
	}

	for (i = 0; i < NXCONF; i++)
	{
	  j = tmpuser->xconf[i].usernum;
	  if (!j || (tmpname = getusername(j, 0)))
          {
	    if (!j || strcmp(name, tmpname) < 0)
	    {
	      for (j = NXCONF - 1; j > i; j--)
	        tmpuser->xconf[j] = tmpuser->xconf[j - 1];
	      tmpuser->xconf[i].usernum = up->usernum;
	      tmpuser->xconf[i].which = which;
	      unlocks(SEM_USER);
	      tmpuser->xconftime = time(0);
	      my_printf("\n%s %s.\n", name, which == 'E' ? "enabled" : "disabled");
	      break;
	    }
	    else if (!strcmp(name, tmpname))
	    {
	      unlocks(SEM_USER);
	      my_printf("\n%s is already in enable/disable list.\n", name);
	      break;
	    }
          }
	}

	if (i == NXCONF)
	{
	  unlocks(SEM_USER);
	  errlog("SNH %s %s", __FILE__, __LINE__);
	}
      }


      else
      {
	locks(SEM_USER);

	for (i = 0; i < NXCONF && tmpuser->xconf[i].usernum; i++)
	  if (up->usernum == tmpuser->xconf[i].usernum)
	    break;

	if (i == NXCONF || !tmpuser->xconf[i].usernum)
	{
	  unlocks(SEM_USER);
	  my_printf("\n%s not in list.\n", name);
	}
	else
	{
	  for (j = i; j < NXCONF - 1; j++)
	    tmpuser->xconf[j] = tmpuser->xconf[j + 1];
	  tmpuser->xconf[NXCONF - 1].usernum = 0;
	  unlocks(SEM_USER);
	  tmpuser->xconftime = time(0);
	  my_printf("\n%s removed from list.\n", name);
	}
      }
    }
    else
    {
      my_printf("Quit\n");
      break;
    }
  }
}



void
dokey(struct user *up)
{
  char key[6], mykey[6];
  int i;
  

  if (up != ouruser)
  {
    my_printf("\nUser must do this -- you can hit 'v' instead to force-validate.\n");
    return;
  }

  hit_return_now();
  locks(SEM_USER);
  unlocks(SEM_USER);

  for (i = 0; i < MAXNEWBIES; i++)
    if (!msg->newbies[i].time)
    {
      locks(SEM_NEWBIE);
      if (!msg->newbies[i].time)
      {
        strcpy(msg->newbies[i].name, up->name);
        msg->newbies[i].time = up->time - 60*60*4;
        if (msg->maxnewbie < i)
          msg->maxnewbie = i;
        unlocks(SEM_NEWBIE);
        break;
      }
      unlocks(SEM_NEWBIE);
    }
  if (i == MAXNEWBIES)
    errlog("Newbie list full");
}



struct user *
change_user(void)
{
  struct user *tmpuser = NULL;
  char *name;

  if (!*profile_default)
    my_printf("\nUser -> ");
  else
    my_printf("\nUser (%s) -> ", profile_default);

  name = get_name("", 2);
  if (!*name)
    name = *profile_default ? profile_default : NULL;

  if (!name || !(tmpuser = getuser(name)) || (!strcmp(name, "Guest") && !ouruser->f_prog))
  {
    if (tmpuser)
    {
      my_printf("User not found.\n");
      freeuser(tmpuser);
    }
    return(NULL);
  }

  if (tmpuser->f_prog && !ouruser->f_admin)
  {
    my_printf("Let the programmers do their own configuration, please!\n");
    freeuser(tmpuser);
    return(NULL);
  }

  strcpy(profile_default, name);
  profile(NULL, tmpuser, PROF_ALL);
  return(tmpuser);
}


void
change_doing(void)
{
char doing[50];

  if (ouruser->f_twit)
    return;

  if (*ouruser->doing)
  {
    my_printf ("Old doing was: %s\nDo you wish to change this? ", ouruser->doing);
    if (yesno(-1) == NO)
      return;
  }
  else
    my_printf ("Enter a new doing for yourself.\n");

  colorize (">@C");
  get_string("", sizeof (ouruser->doing) - 1, doing, -1);
  locks (SEM_USER);
  strcpy (ouruser->doing, doing);
  unlocks (SEM_USER);
  doingchange (doing);
}


void
doingchange (const char *doing)
{
int size;
int count;

  size = count = 0;
  const char* p = doing;
  while (*p && count < 27)	/* 27 spaces in the wholist */
  {
    if (*p == '@' || *(p - 1) == '@')
      size++;
    else
      count++;
    p++;
  }
  bzero (mybtmp->doing, sizeof (mybtmp->doing));
  strncpy (mybtmp->doing, doing, size + count);
}


void
assignquickx (int slot, struct user *tmpuser)
{
char *newname;
struct user *olduser = NULL;
struct user *up = NULL;

  if (tmpuser->quickx[slot])
    olduser = finduser (NULL, tmpuser->quickx[slot], 0);

  if (!olduser)
    my_printf ("QuickX slot #%d> ", slot);
  else
    my_printf ("QuickX slot #%d (%s)> ", slot, olduser->name);

  newname = get_name ("", 2);
  if (!*newname && olduser)
  {
    my_printf ("Delete %s from slot #%d ? ", olduser->name, slot);
    if (yesno(-1) == YES)
    {
      locks (SEM_USER);
      tmpuser->quickx[slot] = 0L;
      unlocks (SEM_USER);
    }
  }
  else 
  {
    if (!(up = getuser (newname)) || (up->f_invisible))
    {
      my_printf ("User does not exist\n");
      return;
    }

    locks (SEM_USER);
    tmpuser->quickx[slot] = up->usernum;
    unlocks (SEM_USER);
  }
}


void
do_bigzap (struct user *tmpuser)
{
int i;
int c;


  my_printf ("Would you like to <z>ap or <u>nzap all public rooms?  (<enter> to cancel) -> ");
  c = get_single_quiet ("ZU Q\n");
  if (strchr ("Q \n", c))
    return;

  if (ouruser->f_novice)
  {
    my_printf ("\nAre you sure? ");
    if (yesno (-1) == NO)
      return;
  }

  /*
   * Run down the list of rooms.  Can't zap the lobby nor
   * can we zap mail> so start the loop with rm #2.
   *
   * Only update rooms that are in use and non-private
   * (remember guessname and passwd rooms have that bit set)
   */


  locks (SEM_USER);
  if (c == 'U')
  {
    for (i = 2; i < MAXROOMS; i++)    
      if (msg->room[i].flags & QR_INUSE
          && !(msg->room[i].flags & QR_PRIVATE)
          && tmpuser->generation[i] != RODSERLING)
      {
	tmpuser->generation[i] = msg->room[i].gen;
	tmpuser->forget[i] = TWILIGHTZONE;
      }
  }
  else if (c == 'Z')
  {
    for (i = 2; i < MAXROOMS; i++)
      if (msg->room[i].flags & QR_INUSE
          && !(msg->room[i].flags & QR_PRIVATE)
          && tmpuser->generation[i] != RODSERLING) 
      {
        tmpuser->forget[i] = msg->room[i].gen;
        tmpuser->generation[i] = TWILIGHTZONE;
      }
  }  
  unlocks (SEM_USER);
}


void
change_vanityflag (struct user *tmpuser)
{
char flag[50];

  if (*tmpuser->vanityflag)
  {
    my_printf ("\nYour current flag is: %s\n", tmpuser->vanityflag);
    my_printf ("Do you wish to change this? ");
    if (yesno(-1) == NO)
      return;
  }
  else
   my_printf ("Enter a nice vanityflag for yourself.\n");

  get_string (">", sizeof (tmpuser->vanityflag) - 1, flag, -1);

  if (*flag)
  {
    locks (SEM_USER);
    checked_snprintf(tmpuser->vanityflag,
        sizeof (tmpuser->vanityflag), "%s", flag);
    unlocks (SEM_USER);
  }

}
