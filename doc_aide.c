/*
 * doc_aide.c - Handles functions of the sysop (formerly aide) menu.
 */
#include "defs.h"
#include "ext.h"

static void aide_logout(void);
static void createroom(void);
static void deleteroom(void);
static void editdesc(void);
static void editroom(void);
static void invite(void);
static void kickout (void);
static void logout_all (void);
static void whoknows(void);


/**********************************************************************
* aide menu
* Access to this menu is restricted to those with aide status.
* Wizards can do everything
* System Aides can do all but aideify
* Room Aides can only get here if it's their room and they can
* NOT do <A>ideify <C>reateroom <Z>ap room.
**********************************************************************/
void
aide_menu(void)
{
int chr = '?';

  /* Can't get here unless you're at least a room aide for this room */

  while (chr != 'M' && chr != ' ' && chr != '\n')
  {
    switch (chr)
    {
//      case 'B':
//        if (ouruser->f_prog || ouruser->f_admin)
//          system("/bin/bash -il");
//       else
//        chr = 0;
//      break;
	
      case 'C':
	my_printf("Create new forum\n");
	createroom();
	loadroom();
	break;

      case 'D':
	my_printf("Edit forum description\n\n");
	editdesc();
	break;

      case 'E':
	my_printf("Edit forum\n");
	editroom();
	break;

      case 'H':
	my_printf("Help!\n");
	if (ouruser->f_admin)
	  help("aidemenu", YES);
	else
	  help("fmmenu", YES);
	break;

      case 'I':
	my_printf("Invite user\n");
	invite();
	break;

      case 'K':
	my_printf("Kick out user\n");
	kickout();
	break;

      case 'L':
	if (ouruser->f_admin)
	{
	  my_printf("Logout all users\n");
	  logout_all();
	  break;
	}
	/* FALL THRU */

      case 'l':
        my_printf("Logout user\n");
        aide_logout();
        break;

      case 'O':
	show_online(1);
	break;

      case 'S':
        searchtool();
        break;

      case 'W':
	whoknows();
	break;

      case 'X':
	if (ouruser->f_prog || ouruser->f_admin)
	  xbroadcast();
	else
	  chr = 0;
	break;

      case 'Z':
	my_printf("Zap (delete) forum\n");
	deleteroom();
	loadroom();
	break;

      case '?':
      case '/':
	if (ouruser->f_admin)
	  help("aidecmd", NO);
	else
	  help("fmcmd", NO);
	break;

      default:
	break;
    }

    if (chr)
      colorize("\n@Y[%s]@M Forum command ->@G ", msg->room[curr].name);
    if (ouruser->f_admin)
      chr = get_single_quiet("BCDEHIKlLMOSWXZ* \n/?");
    else
      chr = get_single_quiet("DEHIKMW \n/?");

  }				/* end of while loop */
  my_printf("Forum level\n");
}


/**********************************************************************
* createroom
**********************************************************************/
static void
createroom(void)
{
int i;
int     found;
char   *newroom;
char    opt;
int     rm_nbr;
int    qpos;
char pas[11];

  if (ouruser->f_novice)
  {
    my_printf("Are you sure? (Y/N) ->");
    if (!yesno(-1))
      return;
  }
  newroom = get_name("Name for new forum? ", 3);

  if (!*newroom)
    return;

  found = NO;

  /* Simultaneously search for a duplicate and the first unused room */
  for (qpos = rm_nbr = 0; rm_nbr < MAXROOMS && !found; ++rm_nbr)
  {
    /* quit searching if you find a duplicate */
    if ((msg->room[rm_nbr].flags & QR_INUSE)
	&& !strcmp(msg->room[rm_nbr].name, newroom))
      found = YES;

    /* get position of FIRST available room */
    if (!(msg->room[rm_nbr].flags & QR_INUSE) && !qpos)
      qpos = rm_nbr;
  }

  if (found)
  {
    my_printf("There is already a forum by that name\n");
    return;
  }

  /* qpos never gets set if all rooms are in use */
  /* the rest of the function depends on qpos being set -sf */
  if (!qpos)
  {
    my_printf("Sorry, no space is available for another forum.\n");
    return;
  }

  /* Set the access for the new room */
  help("roomaccess", NO);

  my_printf("\n<1>Public <2>Guess name <3>Passworded <4>Private\n");
  my_printf("Enter forum type-> ");
  opt = get_single_quiet("1234");

  my_printf("\n\n\042%s\042, will be a", newroom);

  switch (opt)
  {
    case 1:
      my_printf(" public");
      break;
    case 2:
      my_printf(" guess-name");
      break;
    case 3:
      my_printf(" passworded");
      break;
    case 4:
      my_printf(" private");
      break;
  }

  my_printf(" forum\n");

  my_printf("Install it? (y/n) -> ");
  if (!yesno(-1))
  {
    my_printf("Create Forum aborted\n");
    return;
  }

  if (opt == 3)
  {
    get_string ("Enter Password: ", -10, pas, -1);
  }
  
  /* delete & zero room info & whoknows files if they exist */
  /* NOTE: Need exclusive access here! */
  // TODO: there's a better way to do this.
  {
      char * filename = my_sprintf(NULL,"%sroom%d", DESCDIR, qpos);
      unlink(filename);
      open(filename, O_WRONLY | O_CREAT, 0640);
      free(filename);
  }
  {
      char * filename = my_sprintf(NULL,"%srm%d", WHODIR, qpos);
      unlink(filename);
      open(filename, O_WRONLY | O_CREAT, 0640);
      free(filename);
  }

  locks(SEM_MSG);

  if (msg->room[qpos].flags & QR_INUSE)
  {
    unlocks(SEM_MSG);
    my_printf("\nForum slot taken, please try again.\n");
    return;
  }

  curr = qpos;

  strcpy(msg->room[curr].name, newroom);

  msg->room[curr].highest = 0L;
  msg->room[curr].posted = 0L;

  if (++msg->room[curr].gen == 100)
    msg->room[curr].gen = 10;

  /* want a clean slate to work with */
  msg->room[curr].flags = QR_INUSE;

  if (opt > '1')
    msg->room[curr].flags |= QR_PRIVATE;
  if (opt == '2')
    msg->room[curr].flags |= QR_GUESSNAME;
  if (opt == '3') 
  {
    msg->room[curr].flags |= QR_PASSWORD;
    strcpy (msg->room[curr].passwd, pas);
  }

  for (i = 0; i < MSGSPERRM; i++)
  {
    msg->room[curr].pos[i] = 0;
    msg->room[curr].num[i]= 0;
    msg->room[curr].chron[i]= 0;
  }

  unlocks(SEM_MSG);

  my_printf("Use edit description to assign forum moderator.\n");

  my_printf("\n%s> (#%d) created as a", msg->room[curr].name, curr);

  if (!(msg->room[curr].flags & QR_PRIVATE))
    my_printf(" public");

  if (msg->room[curr].flags & QR_PRIVATE)
    my_printf(" [private]");

  if (msg->room[curr].flags & QR_GUESSNAME)
    my_printf(" [guessname]");

  if (msg->room[curr].flags & QR_PASSWORD)
    my_printf(" [passworded]");

  my_printf(" forum\n");

  /* Join user to the room that was just created */
  ouruser->forget[curr] = TWILIGHTZONE;
  ouruser->generation[curr] = msg->room[curr].gen;

  return;
}


/**********************************************************************
* deleteroom
**********************************************************************/
static void
deleteroom(void)
{
char    confirm[7];

  if (curr <= AIDE_RM_NBR)
  {
    my_printf("Can't kill this forum.\n");
    return;
  }

  get_string("Type 'DELETE' to kill this forum -> ", 6, confirm, -1);
  if (strcmp(confirm, "DELETE"))
    return;

  my_putchar('\n');


  locks(-1);
  msg->room[curr].flags = 0;

  char * filename = my_sprintf(NULL,"%sroom%d", DESCDIR, curr);
  unlink(filename);
  free(filename);

  filename = my_sprintf(NULL,"%srm%d", WHODIR, curr);
  unlink(filename);
  free(filename);
  unlocks(-1);

  /* you just nuked the room, need to go somewhere! */
  curr = LOBBY_RM_NBR;
}


/**********************************************************************
* edit room description
* Ask user if they want to edit <R>oom aide only, <D>escription only,
* <B>oth, or <Q>uit.
* Contingent on option chosen, edit the item(s).  When done, write
* the description to the descfile.
* raname is taken either from reading the description or set anew.
* If a new aide is chosen, read his/her user file and assign ->usernum
* to be roomaide.
**********************************************************************/
static void
editdesc(void)
{
char    choice = '0';
int     dfd;		/* desc file descriptor */
int     dummy;		/* readmsg() needs this: returns YES/NO */
int     err;		/* makemessage returns this */
char    raname[MAXALIAS+1];
int	upload;
char   *cp;
struct mheader *mh;
int size;
unsigned char *infop;
struct user *tmpuser;

  char * descfile = my_sprintf(NULL,"%sroom%d", DESCDIR, curr);
  char * newdescfile = my_sprintf(NULL,"%sroom%d.NEW", DESCDIR, curr);

  size = 0;
  if (!(infop = (unsigned char *)mymmap(descfile, &size, 0)) || !size)
  {
    colorize("@RDescription doesn't yet exist@G\n");
    choice = 'B';
  }
  else
  {
    readmessage(infop, &dummy, raname, FALSE, 0);
    munmap((void *)infop, size);
  }

  /* if it's a new description, don't bother prompting for choice */
  if (choice != 'B')
  {
    my_printf("\nEdit <F>orum moderator only, <D>escription only, <B>oth, <Q>uit -> ");
    choice = get_single_quiet("FDBQ \n");
    my_putchar('\n');
  }
  if (choice == 'Q' || choice == ' ' || choice == '\n')
    goto Return;

  if (choice == 'B' || choice == 'F')
  {
    cp = get_name("\nEnter 'Sysop' for default administrator moderation.\nNew forum moderator -> ", 2);
    if (!*cp)
      goto Return;

    if (!strcmp(cp, "Sysop"))
      msg->room[curr].roomaide = 0;
    else
      if (!(tmpuser = getuser(cp)) || tmpuser->f_invisible)
      {
	if (tmpuser)
	  freeuser(tmpuser);
	my_printf("\nThere is no user %s on this BBS.\n", cp);
        goto Return;
      }
      else
      {
        msg->room[curr].roomaide = tmpuser->usernum;
	freeuser(tmpuser);
      }

    if (choice == 'F')
      goto Return;
  }

  my_printf("\nHit Y to upload a description or N to enter it normally (Y/N) -> ");
  if ((upload = yesno(-1)))
    my_printf("\n(Use control-D to end!)\n");

  my_printf("\nEnter a new forum description...\n\n");

  err = makemessage(NULL, MES_DESC, upload);

  if (err == ABORT)
    colorize("@RDescription not entered\n@G");
  else if (err != SAVE)
    colorize("@RSome mystical error - can't make description\n@G");
  else if ((dfd = open(newdescfile, O_WRONLY | O_CREAT | O_EXCL, 0640)) < 0)
    my_printf("error opening desc file to make final copy\n");
  else
  {
    mh = (struct mheader *)(void *)tmpstart;
    write(dfd, tmpstart, mh->hlen + mh->len + 1);
    close(dfd);
    rename(newdescfile, descfile);
  }
  munmap((void *)tmpstart, 53248);
  if (err == SAVE)
  {
    my_printf("\nMark forum info as having been updated? (Y/N) -> ");
    if (yesno(-1))
      msg->room[curr].descupdate = msg->room[curr].highest;
  }

Return:; // free the mallocs
    free(descfile);
    free(newdescfile);
}


/**********************************************************************
* editroom
* <E>dit room in aide menu
**********************************************************************/
static void
editroom(void)
{
char    anon_opt;
char    opt;
char   *roomname;
char	pas[11];
int     flags;
int     gen;


  if (curr <= AIDE_RM_NBR && !ouruser->f_admin)
  {
    my_printf("Only programmers can edit Lobby, Mail, or Aide forums!\n");
    return;
  }

  flags = msg->room[curr].flags;
  gen = msg->room[curr].gen;

  /* Edit the room name */
  my_printf("Forum name is currently: %s>\n", msg->room[curr].name);

  roomname = get_name("New forum name (return leaves it unchanged) -> ", 3);

  /* If you're an aide or wizard, you can edit the permissions of the room */

  if (ouruser->f_admin)
  {
    /************************************************************
    * These ifs form a sentence onscreen.
    ************************************************************/

    my_printf("\n%s> was a", msg->room[curr].name);

    if (!(msg->room[curr].flags & QR_PRIVATE))
      my_printf(" public");

    if (msg->room[curr].flags & QR_PRIVATE)
      my_printf(" private");

    if (msg->room[curr].flags & QR_GUESSNAME)
      my_printf(" guessname");

    if (msg->room[curr].flags & QR_PASSWORD)
      my_printf(" passworded");

    if (msg->room[curr].flags & QR_ANONONLY)
      my_printf(" Anonymous-only");

    if (msg->room[curr].flags & QR_ANON2)
      my_printf(" Anonymous-optional");

    my_printf(" forum\n");

    my_printf("New forum type <1>pub <2>guessname <3>passworded <4>inv-only -> ");
    opt = get_single_quiet("1234");

    my_putchar('\n');

    /**********************************************************************
    * Option 1 (public) is default.
    * Non-public rooms have only 1 or 2 "privacy" bits: 
    * QR_PRIVATE by itself means invite-only, 
    * otherwise also set QR_GUESSNAME || QR_PASSWORD
    * Turn off all but inuse bits, we're going to reset everything
    * Turn bits back on as we go along
    **********************************************************************/
    flags = QR_INUSE;

    if (opt > '1')
      flags |= QR_PRIVATE;

    if (opt == '2')
      flags |= QR_GUESSNAME;

    if (opt == '3')
      flags |= QR_PASSWORD;

    if (opt != '1')
    {
      my_printf("(Answer yes to start a private forum)\n");
      my_printf("Cause users to forget forum? (y/n) -> ");

      gen = msg->room[curr].gen;
      if (yesno(-1) == YES)
      {
	if (++gen == 100)
	  gen = 10;
      }
    }
  
    if (flags & QR_PASSWORD)
      get_string ("Enter Password (enter for no change): ", -10, pas, -1);
  
  }

  flags &= ~(QR_ANONONLY | QR_ANON2);
  my_printf("Identity: <1>Normal <2>Anon <3>Anon-optional -> ");
  anon_opt = get_single_quiet("123");

  if (anon_opt == '1')
    my_printf("Normal\n");
  else if (anon_opt == '2')
  {
    flags |= QR_ANONONLY;
    my_printf("Anonymous only\n");
  }
  else
  {
    flags |= QR_ANON2;
    my_printf("Anonymous optional\n");
  }

  colorize("@YSave changes? (Y/N) ->@G ");
  if (yesno(-1) == YES)
  {
    locks(SEM_MSG);
    if (*roomname)
      strcpy(msg->room[curr].name, roomname);
    msg->room[curr].flags = flags;
    if (flags & QR_PASSWORD && *pas)
      strcpy(msg->room[curr].passwd, pas);
    msg->room[curr].gen = gen;
    unlocks(SEM_MSG);
  }

  my_printf("\nUse edit <D>escription to change description/forum moderator.\n");
}


/**********************************************************************
* invite
* Modifies the user's generation and forget numbers to make a member
* of the room.
**********************************************************************/
static void
invite(void)
{
struct user *tmpuser;
char   *uname;


  uname = get_name("Name of user to invite? ", 2);

  if (*uname)
  {
    if (!(tmpuser = getuser(uname)) || tmpuser->f_invisible)
    {
      if (tmpuser)
        freeuser(tmpuser);
      my_printf("There is no user %s on this BBS.\n", uname);
      return;
    }
    if (tmpuser->generation[curr] == msg->room[curr].gen || (!(msg->room[curr].flags & QR_PRIVATE) && tmpuser->generation[curr] != RODSERLING))
    {
      freeuser(tmpuser);
      my_printf("%s is already invited to this forum.\n", uname);
      return;
    }

    locks(SEM_USER);
    tmpuser->generation[curr] = msg->room[curr].gen;
    tmpuser->forget[curr] = TWILIGHTZONE;
    unlocks(SEM_USER);

    freeuser(tmpuser);
    my_printf("%s invited to %s>\n", uname, msg->room[curr].name);
  }
}


/**********************************************************************
* kickout
* Edit a user's generation number for a room so (s)he can't get to it.
**********************************************************************/
static void
kickout(void)
{
struct user *tmpuser;
char   *uname;


  if (curr == LOBBY_RM_NBR)
  {
    my_printf("Get a clue!  You can't kick a user out of the Lobby!\n");
    return;
  }

  uname = get_name("Name of user to kick out? ", 2);

  if (*uname)
  {
    if (!(tmpuser = getuser(uname)) || tmpuser->f_invisible)
    {
      if (tmpuser)
        freeuser(tmpuser);
      my_printf("There is no user %s on this BBS.\n", uname);
      return;
    }
    else
    {
      if ((tmpuser->generation[curr] == ((msg->room[curr].flags & QR_PRIVATE) ? TWILIGHTZONE : RODSERLING)) && tmpuser->forget[curr] == TWILIGHTZONE)
      {
        freeuser(tmpuser);
	my_printf("%s doesn't belong to this forum.\n", uname);
	return;
      }
      locks(SEM_USER);
      tmpuser->generation[curr] = ((msg->room[curr].flags & QR_PRIVATE) && !(msg->room[curr].flags & QR_GUESSNAME)) ? TWILIGHTZONE : RODSERLING;

      tmpuser->forget[curr] = TWILIGHTZONE;
      tmpuser->generation[curr] = RODSERLING;

      unlocks(SEM_USER);
      freeuser(tmpuser);

      my_printf("%s is now kicked out of %s>\n", uname, msg->room[curr].name);
    }
  }
}


static void
aide_logout(void)
{
struct user *tmpuser;
char   *uname;
struct btmp btmp;

  uname = get_name("Name of user to log out? ", 2);
 
  if (*uname){
    if (!(tmpuser = getuser(uname)))
      my_printf("There is no user %s on this BBS.\n", uname);
    else
    {
      if (tmpuser->f_prog && !ouruser->f_admin)
        my_printf("Can't do that!\n");
      else if (!is_online(&btmp, tmpuser, NULL))
        my_printf("%s is not online.\n", uname);
      else
	logout_user(tmpuser, NULL, 0);
      freeuser(tmpuser);
    }
  }
}

static void
logout_all(void)
{
  int i;

  my_printf("Are you are you want to logout all users on the BBS? (Y/N) -> ");
  if (yesno(-1))
  {
    my_printf("\nLogging off all users...");
    fflush(stdout);
    for (i = 0; i < MAXUSERS; i++)
      if (bigbtmp->btmp[i].pid != pid)
        logout_user(NULL, &bigbtmp->btmp[i], 0);
    sleep(10);
    my_printf("\n\nForcing off any stragglers...");
    fflush(stdout);
    for (i = 0; i < MAXUSERS; i++)
      if (bigbtmp->btmp[i].pid != pid)
        logout_user(NULL, &bigbtmp->btmp[i], 1);
    sleep(5);
    my_putchar('\n');
  }
}



/**********************************************************************
* whoknows
* Simply mores the whoknows list for current room
* update aide list is called after an aide change
**********************************************************************/
static void
whoknows(void)
{
  char * filename = my_sprintf(NULL,"%srm%d", WHODIR, curr);
  more(filename, 0);
  free(filename);
}


