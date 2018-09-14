/*
 *  doc.c - Handles function of the main forum prompt.
 */
#include "defs.h"
#include "ext.h"
/*char fun_stuff[][80] = {
	"IM IN UR LOBBY!! EATIN UR PROMPTZ!!!",
	"Aliens are attacking, log off now.",
	"YAY BABY JESUS!",
	"MA PETITE FRENCH TOAST!!",
	"I swallowed a bug.",
	"You type like I drive.",
	"Eat me, I'm a Danish.",
	"Got too much time on your hands?  Answer today's poll in Polls>!",
	"The Internet is being cleaned.  Please log off.",
	"UNGH!!",
	"Tisket!",
	"You just lost The Game.",
	"Do you have stairs in your house?",
	"Shut up, Fitty.",
	"My everything hurts! -Fibromyalgia Joe",
	".-( BOO, motherfucker!  I'm a dot! )",
	"Here ya go, Shaq.  Go buy yo-self a bouncin' car!",
	"{o,o} - ( HOOOOOOT! )",
	"8=====D",
	"({})",
	"(.Y.)",
	"BEW!BEW!BEW!",
	"Inara BBS has a Literature> forum, for book discussion.",
	"*poot*",
	"If sick, dump tubing.",
	"You are incorrect, as usual.",
	"Joni Mitchell still sucks it.",
	"Thanks for not peeing on my furniturez!",
	"IM IN UR FRIDGE    EATIN UR FOODZ",
	"Bleeding Me is made of matzoh and sand.",
	"There is no spoon.",
	"I can kill you with my brain.",
	"I am a leaf on the wind.  Watch how I soar.",
	"ZOMG!",
	"Fat people are lonely...come talk to us in Chub Club>.",
	"Mine is an evil laugh. Now DIE!",
	"Forget your toothpaste?  *PUNCH*",
	"All your prompt are belong to us.",
	"Twofry...Nice hat.  *POP*",
	"My food is problematic.",
	"No power in the 'verse can stop me.",
	"1+1=Lesbian",
	"Is this Toledo?",
	"Try the Weird>, it's delicious!",
	"Computer trouble? Jump to Computer Geekery>.",
	"WinAMP - it really whips the llama's ass.",
	"Lord.",
	"Go to Photography> to talk about photography!",
	"Laughs",
	"Inara Radio: http://www.inararadio.com/",
	"What?",
	"Goto",
	"Somebody needs a nap.",
	"Inara BBS: Now with 20% more JoJo",
	"B'gawwwwwwk! ~:>",
	"Got an idea for Inara BBS?  <j>ump to Ideas> and post it.",
	"<insert gay Sysop joke>",
	"*blank stare*",
	"BINGO!",
	":)",
	"Meat for President.",
	"YES.  The plane takes off.",
	"I don't get it.",
	"Is everything okay at home?",
	"Erin has a football head.",
	"That's what SHE said!",
	"What did you change?",
	"You're so vain, you probably think this Goto is about you.",
	"You just got knocked-up by Freef.",
	"SHE HIT THE FLO (she hit the flo)",
	"Be aware of your tongue.",
	"Well, it was just *sitting* there...",
	"*bzownt!*",
	":D",
	"Whore!",
	"Clearly, you suck at life.",
	"Where da white wimmenz at?",
	"This place is dying.  :(",
	"Beans and franks!",
	"I thought you people were supposed to be jolly?",
	"Okay.",
	"I will kill you with my teacup.",
	"OMG PEANUTS!",
	"Wait...what?",
	"What an unfortunate image.",
	"J1ZZBIB!",
	"Looby>",
	"Suck it? *waggles*",
	"Shaka...when the walls fell.",
	"Mandingo!",
     "I like my ravioli hot. Hot like a motherfucker!"
};
int fun_len = sizeof(fun_stuff) / sizeof(char[80]);
*/
/*
 * getseed 
 *    get a seed (just 32 bits) for the pseudorandom generator, using
 *    the simplest possible thing (time returned by gettimeofday()).
 */
static long getseed(void)
{
    struct timeval tp; 
    struct timezone tzp;
    if (gettimeofday(&tp, &tzp)) { 
       return 0;
    }
    return tp.tv_usec;
}

int rnd(int n)
{
  static int first_time=1;
  static char state[256];                                   
  if (first_time) {                                        
    first_time = 0;                                       
    unsigned int seed = (unsigned int) getseed();
    initstate(seed, state, 256);                           
  }
  if (n<0) { return 0; }
  unsigned long mask = 1;
  while (mask<n) mask = 2*mask+1;
  for (;;) {
    long r = random();
    if ((r&mask) < n) return r&mask;
  }
} 

void
bbsstart(void)
{
unsigned char stdinbuf[STDINBUFSIZ];
long    uglastmsg;	/* last msg seen in prev. rm */
long    ugtemp = TWILIGHTZONE;
short   prev_rm = TWILIGHTZONE;
char    cit_cmd;
char    bueller = 0;

  room = &sroom;
  setvbuf(stdin, stdinbuf, _IOFBF, STDINBUFSIZ);
  setvbuf(stdout, stdoutbuf, _IOFBF, STDOUTBUFSIZ);

  init_system();

  reserve_slot();
  do_login();

  colorize("\n@G");

  curr = LOBBY_RM_NBR;
  inituser();
  openroom();
  storeug(&uglastmsg, &ugtemp);

  /* The first thing we do is make the user read the lobby */
  cit_cmd = 'N';

  readroom(cit_cmd);


  for(;;)
  {
    /*
     * check if user has been kicked out of this room while they were in it,
     * or if room was deleted
     */
    if (ouruser->generation[curr] < 0 || !msg->room[curr].flags)
    {
      curr = LOBBY_RM_NBR;
      openroom();
      storeug(&uglastmsg, &ugtemp);
    }

    if (cit_cmd)
      colorize("\n@Y%s>@G ", msg->room[curr].name);

    checkx(0);

    if (ouruser->f_prog)
      cit_cmd = get_single_quiet("0123456789ABbCdD\005eEFfGHIJKLNOpPqQRsStTuUvVwWxX\027\030yYZ !+/?#%@-\"");
    else if (ouruser->f_aide)
      cit_cmd = get_single_quiet("0123456789ABbCdD\005eEFfGHIJKLNOpPqQRsStTuUvVwWxX\027\030yYZ /?#%@-\"");
    else if (ouruser->usernum == msg->room[curr].roomaide && !ouruser->f_twit)
      cit_cmd = get_single_quiet("0123456789ABbCdD\005eEFfGHIJKLNOpPqQRsStTuUvwWxX\027\030yYZ /?#%@-\"");
    else
      cit_cmd = get_single_quiet("0123456789BbCedDEFfGHIJKLNOpPqQRsStTuUvwWxX\027\030yYZ /?#%@-\"");

    if (cit_cmd == SP)
      cit_cmd = 'N';

    if (guest && !strchr("0123456789BbFGHIJKLNOpPRsSTUwWyY/?#-", cit_cmd))
    {
      colorize("\n\n@RThe Guest user cannot do that.@G\n");
      continue;
    }

//    if (curr == LOBBY_RM_NBR && strchr("dDGNqsTU\027X\030Z% ", cit_cmd))
//    {
//      if (bueller++ >= 12) 
//     {
//	int rndf = rnd(fun_len);
//	colorize("@R%s@Y\nLobby> @G", fun_stuff[rndf]);
//	flush_input(bueller / 25);
//	bueller = 0;
  //   }
//      if (bueller >= 100)
//      {
//        colorize("@R\n\n\nGo away until you have something useful to do!\n\n\n@G");
//        my_exit(10);
//      }
//    }
//    else
//      bueller = 0;

    if (strchr("0123456789AC\005eEHJpPQSvVx\030yYZ#-\"", cit_cmd))
      mybtmp->nox = 1;

    switch (cit_cmd)
    {
      case '+':
	system("/home/bbs/bin/console.pl");
	//debug();
        break;
      case '!':
        my_printf("Block host\n");
        blockhost();
        break;
//      case '.':
//	system("/home/bbs/bin/dotmenu.sh");
        //help("dotkey", NO);
//        break;

      case 'A':
	my_printf("Sysop commands\n", msg->room[curr].name);
	aide_menu();
	break;

      case 'B':
        change_beeps();
        break;

      case 'R':
      case 'b':
	cit_cmd = 'R';
	my_printf("Read Reverse\n");
	readroom(cit_cmd);
	break;

      case 'C':
        my_printf("Change config\n");
        change_setup(NULL);
	break;
      case 'd':
	my_printf("Change doing field\n\n");
        change_doing();
       break; 
      case 'D':
	show_online(4);
	break;

      case '\005':
	if (ouruser->usernum == msg->room[curr].roomaide)
	{
	  my_printf("Enter Forum Moderator message\n\n");
	  if (ouruser->f_novice)
	  {
	    my_printf("Are you sure you want to enter a message as Forum Moderator? (Y/N) -> ");
	    if (!yesno(-1))
	      break;
	  }
	  sysopflags |= SYSOP_FROM_FM;
	}
	else if (ouruser->f_admin)
        {
	  my_printf("Enter Sysop message\n\nNOTE: You are entering this message as Sysop!\n\n");
	  sysopflags |= SYSOP_FROM_SYSOP;
        }
	/* FALL THRU */

      case 'e':
      case 'E':
	{
	  char work[20];

	  if (ouruser->f_newbie && (curr == MAIL_RM_NBR || curr > 4))
	    help("newuseraccess", NO);
	  else
	  {
	    if (cit_cmd == 'E')
	      my_printf("Upload message\n\n");
	    else if (cit_cmd == 'e')
	      my_printf("Enter message\n\n");
	    *work = 0;
	    if (entermessage(curr, work, cit_cmd == 'e' ? 0 : ouruser->f_novice ? 2 : 1))
	      alt = TRUE;
	    sysopflags &= ~(SYSOP_FROM_SYSOP | SYSOP_FROM_FM);
	  }
	}
	break;

      case 'f':
	my_printf("Read Forward\n");
	readroom(cit_cmd);
	break;

      case 'F':
        my_printf ("Fortune\n\n");
        do_fortune();
        break;

      case 'G':
	alt = TRUE;
	my_printf("Goto ");
	updatels(&prev_rm);
	/* find next room with unread msgs and open it */
	nextroom();
	openroom();
	storeug(&uglastmsg, &ugtemp);
	break;

      case 'H':
	my_printf("Help!\n");
	help("topics", YES);
	break;

      case 'q':
      case 'Q':
        get_syself_help(cit_cmd);
        break;

      case 'I':
	my_printf("Forum Info\n");
	readdesc();
	break;

      case 'J':
	{
	  int old_rm;

	  my_printf("Jump to ");
	  old_rm = curr;
	  if (findroom() == YES)
	  {
	    int save_rm;
  
            mybtmp->nox = 0;
	    save_rm = curr;
	    curr = old_rm;
	    alt = TRUE;
	    updatels(&prev_rm);
	    curr = save_rm;
	    openroom();
	    storeug(&uglastmsg, &ugtemp);
	  }
	}
	break;

      case 'K':
	my_printf("Known forums and zapped list\n");
	knrooms(ouruser);
	break;

      case 'L':
        dologout();
	break;
	
      //case 'M':
	//system("/home/bbs/bin/iscawholist");
        //more (ISCAWHO, 0);
	//break;

      case 'N':
	if (!alt && (ouruser->f_clear))
	{
	  ouruser->lastseen[curr] = room->num[MSGSPERRM - 1];
	}

	if (ouruser->lastseen[curr] < room->num[MSGSPERRM - 1])
	{
	  my_printf("Read New\n");
	  readroom(cit_cmd);
	}
	else
	{			/* No new notes so just do a Goto now */
	  my_printf("Goto ");
	  alt = TRUE;
	  updatels(&prev_rm);
	  /* find next room with unread msgs and open it */
	  nextroom();
	  openroom();
	  storeug(&uglastmsg, &ugtemp);
	}
	break;

      case 'O':
	my_printf("Read Old messages reverse\n");
	readroom(cit_cmd);
	break;

      case 'p':
      case 'P':
	profile_user(cit_cmd == 'P');
	break;

      case 's':		/* don't update lastseen, you're skipping the room */
	alt = TRUE;
	my_printf("Skip %s\n", msg->room[curr].name);
	skipping[curr >> 3] |= 1 << (curr & 7);
	/* after skipping a room, find the next unread room (not a goto) */
	nextroom();
	openroom();
	ugtemp = ouruser->lastseen[curr];
	break;

      case 'S':
	{
	  int old_rm;

	  my_printf("Skip %s to ", msg->room[curr].name);
	  old_rm = curr;
	  if (findroom() == YES)
	  {
            mybtmp->nox = 0;
	    alt = TRUE;
	    skipping[old_rm >> 3] |= 1 << (old_rm & 7);
	    openroom();
	    ugtemp = ouruser->lastseen[curr];
	  }
	}
	break;
      case 't':
        printdate("Time\n\n%s");
        break;

      case 'T':
	my_printf("OMG DEY CUT OFF MAH TWEETER!\n");
        //twitter();
	break;

      case 'u':
	my_printf("Ungoto\n");
	alt = TRUE;
	ungoto(prev_rm, &uglastmsg, &ugtemp);
	break;

      case 'U':
        my_printf("Update Facebook Status, coming soon...\n");
        //system("/home/bbs/bin/bbsfb.sh");
	break;

      case 'v':
	express (-1);
	break;

      case 'V':
	my_printf("Validate new users\n");
	validate_users(1);
	break;

      case '\027':
	if (client)
	  clientwho();
	else
	  cit_cmd = 0;
	break;

      case 'W':
	if (ouruser->f_shortwho)
	  show_online(0);
	else
	  show_online(3);
	break;

      case 'w':
	if (ouruser->f_shortwho)
	  show_online (3);
	else
	  show_online(0);
	break;

      case 'x':
	express(10);
	break;

      case 'X':
	change_express(1);
	break;

      case CTRL_X:
	old_express();
	break;

      case 'y':
      case 'Y':
        if (!wanttoyell(cit_cmd))
          break;
	(void)entermessage(-1, "", cit_cmd == 'y' ? 0 : ouruser->f_novice ? 2 : 1);
	break;

      case 'Z':
	my_printf("Zap forum\n");
	if (forgetroom())
        {
	  alt = TRUE;
	  nextroom();
	  openroom();
	  ugtemp = ouruser->lastseen[curr];
        }
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
        express (cit_cmd - '0');
        break;

      case '?':
      case '/':
	if (guest)
	  help("guestforumlevel", NO);
	else
	  help("doccmd", NO);
	break;

      case '#':
	readroom(cit_cmd);
	break;

      case '%':
	if (ouruser->f_elf && !ouruser->f_restricted && !ouruser->f_twit)
          if (mybtmp->xstat && !mybtmp->elf)
            my_printf("\n\nYou can't enable yourself as a guide while your X's are disabled.\n");
	  else if (mybtmp->elf = !mybtmp->elf)
	    my_printf("\n\nYou are now marked as being available to help others.\n");
	  else
	    my_printf("\n\nYou are no longer marked as being available to help others.\n");
	else
	  cit_cmd = 0;
	break;

      case '-':
	readroom(cit_cmd);
	break;

      case '@':
	my_printf("Aidelist\n");
	more(AIDELIST, 0);
	break;

//      case '*':
//	my_printf("Aidelist\n");
//	more(NEWAIDELIST, 0);
//        break;
	 
      case '"':
	{
	  char work[20];

	  my_printf("Quote X messages to Sysop\n");
          *work = 0;
          (void)entermessage(-1, work, -1);
	}
	break;

      default:
	break;
    }				/* switch */

  }
}



/*
 * New messages are those with universal message numbers greater than the value
 * in ouruser->lastseen[MAIL_RM_NBR].
 */
int
checkmail(struct user *tmpuser, int  quiet)
{
    int i;
    int     count = 0;

  /* See if user is such a doofus we've kicked them out of Mail> */
  if (tmpuser->generation[MAIL_RM_NBR] == RODSERLING)
    return(NO);

  for (i = MAILMSGS - 1; i >= 0 && tmpuser->lastseen[MAIL_RM_NBR] < tmpuser->mr[i].num; i--)
    if (tmpuser->mr[i].pos > 0)
    {
      if (count++ < 0)
        count++;
    }
    else if (!count)
      count--;

  if (!quiet)
    if (count == 1)
      my_printf("*** You have a new private message in Mail>\n");
    else if (count > 1)
      my_printf("*** You have %d new private messages in Mail>\n", count);

  return(count > 0 ? count : 0);
}



void
help(char *topic, int morehelp)
{
char help_str[30];
char hfile[100];
register int f;
int toast;

  sprintf(hfile, "%s%s", HELPDIR, topic);
  more(hfile, 1);
  if (!morehelp)
    return;

  for (;;)
  {
    colorize("\n@YEnter help topic ->@G ");
    get_string("", 29, help_str, -1);
    if (!*help_str)
      return;
    /* We don't want these people walking the tree */
    if (index(help_str, '.') || index(help_str, '/'))
      continue;

    for (toast = 0; help_str[toast] != '\0' && toast < 30; toast++) {
      help_str[toast] = tolower(help_str[toast]);
    }
//    *help_str = tolower(*help_str);
    sprintf(hfile, "%s%s%s", HELPDIR, "topics.", help_str);

//    colorize("\n@WDebug: %s@G\n", hfile);

    if ((f = open(hfile, O_RDONLY)) < 0)
      colorize("\n@RTopic not found.@G");
    else
    {
      close(f);
      more(hfile, 1);
    }
  }
}



/*
 * Flag rooms a user no longer belongs to so generation numbers are kept
 * consistent.  Also resets any pointers that might have gotten out of range.
 */
void
inituser(void)
{
register int i;

  for (i = 0; i < MAXROOMS; ++i)
  {
    if (ouruser->generation[i] != msg->room[i].gen &&
        ouruser->generation[i] != RODSERLING)
      ouruser->generation[i] = TWILIGHTZONE;

    if (ouruser->forget[i] != msg->room[i].gen &&
        ouruser->forget[i] != NEWUSERFORGET)
      ouruser->forget[i] = TWILIGHTZONE;

    if (i != MAIL_RM_NBR)
    {
      if (ouruser->lastseen[i] > msg->room[i].highest)
        ouruser->lastseen[i] = msg->room[i].highest;
    }
    else if (ouruser->lastseen[MAIL_RM_NBR] > ouruser->mr[MAILMSGS - 1].num)
      ouruser->lastseen[MAIL_RM_NBR] = ouruser->mr[MAILMSGS - 1].num;
  }
}



int
wanttoyell(int cmd)
{
  my_printf("%s", cmd == 'y' ? "Yell to Sysop\n" : "Upload Yell to Sysop\n");
  help("yell.list", NO);
  my_printf("Enter your choice -> ");
  switch (get_single_quiet("1234YN \n"))
  {
    case '1':
      help("yell.voice.1", NO);
      break;
    case '2':
      help("yell.voice.2", NO);
      break;
    case '3':
      help("yell.voice.3", NO);
      break;
    case '4':
      help("yell.voice.4", NO);
      break;
    case 'Y':
      my_putchar('\n');
      return(1);
    default:
      my_putchar('\n');
      break;
  }
  return(0);
}

void
blockhost(void)
{
char bl_host[51];
char blockhost[125];

colorize("@YHostname or IP to block (blank to quit): @G");
get_string("", 51, bl_host, -1);
if (!strlen(bl_host) == 1)
{
colorize("@YCancel.\n@G");
}
else
{
//printf("Hmm.  Can't block %s", bl_host);
printf("Are you sure you want to block %s(Y/n)? ", bl_host);
switch (get_single_quiet("YyNn\n"))
  {
//    case 'y':
//	printf("\n");
 //       sprintf(blockhost, "/home/bbs/bin/blockip.sh %s", bl_host);
 //       system(blockhost);
//	printf("%s blocked\n", bl_host);
 //     break;

    case 'Y':
        printf("\n");
	sprintf(blockhost, "/home/bbs/bin/blockip.sh %s", bl_host);
	system(blockhost);
        colorize("@R\n%s blocked\n@G", bl_host);
      break;

    case 'N':
colorize("@RCanceled.\n");
      break;
    case 'n':
colorize("@RCanceled.\n");
      break;


    default:
colorize("@RCanceled.\n");
      break;

}
//sprintf(blockhost, "echo \"/home/bbs/bin/blockip.sh %s\"> /tmp/test", bl_host);
//system(blockhost);
//}
}
}

void
twitter(void)
{
char twname[32];
char twpass[16];
char twinput[126];
char tweetpost[200];
char *tweet = malloc(strlen(twinput) +3);
colorize("@YTwitter\n");
colorize("@YYour Twitter username: @G");
get_string("", 29, twname, -1);
colorize("@YYour Twitter password: @G");
get_string("", 16, twpass, -1);
colorize("@YYour Tweet: @G");
get_string("", 70, twinput, -1);

sprintf(tweetpost, "/home/bbs/bin/bbstw.sh %s %s \'%s\'", twname, twpass, twinput);

system(tweetpost);

}

void
dologout(void)
{
  my_printf("Logout\n\nReally log out? (Y/N) -> ");
  flush_input(0);
  if (yesno(-1))
  {
    /*
    my_printf("\nYou are the weakest link.  Goodbye.\n");
    my_printf("\nDude, I think it would be best if we didn't piss off Cartman again.\n");
    */
    my_printf("\n");
    do_fortune();
    my_printf("\nThanks for dropping by.\n");
    my_exit(1);
  }
}
