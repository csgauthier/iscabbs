/*
 * who.c - Handles display of profiles and who list for BBS and finger daemon.
 */
#include "defs.h"
#include "ext.h"


/*
 * show_online() 
 * Show a list of the users online, levels of detail:
 *   0 regular list
 *   1 wizard's debugging info
 *   2 TCP finger port list
 *   3 short list
 */

void
show_online(int     level)
{
struct btmp *btmp;
struct user *tmpuser;
int l;
char    msg_status;
register int     i;
int     tdif,
        min,
        hour,
        pos;
char    work[24];
time_t  curr_time;
long mineternal;
int done = 0;
int whostart, whoend, whoincr;


  if (level != 2 && rows != 32000)
      my_putchar('\n');
  if (bigbtmp->users > 1)
    colorize("There are @Y%d@G users (@Y%d@G queued)", (bigbtmp->users-9) , bigbtmp->queued);
  else
    colorize("There is @Y1@G user (@Y%d@G queued)", bigbtmp->queued-9);

  my_printf("\n\n");

  if (!bigbtmp->users)
    return;
  curr_time = time (0);
  if (level == 1)
    colorize("@YUser Name             @MPID  @Meternal @RTime  @CFrom@G\n");
  else if (level == 4)
    colorize("@YDoing list\n");
  else if (level != 3)
    colorize("@YUser Name              @PTime @RDoing           @W%14s@G\n",
    	formtime (3, curr_time));

  if (level != 3)
    my_printf("----------------------------------------------------------\n");

  if (level == 3)
    l = 2;
  else
    l = 4;
  pos = 0;

  whostart = bigbtmp->users - 1;
  whoend = 0;
  whoincr = -1;

  if ((ouruser != NULL) && ouruser->f_revwho) {
    whoincr = 1;
    whoend = whostart;
    whostart = 0;
  }

  for (mineternal = ((whoincr < 0) ? 999999999 : 0), i = whostart;
			(whoincr < 0) ? (i >= whoend) : (i <= whoend);
			i += whoincr)
  {
    btmp = &bigbtmp->btmp[bigbtmp->index[i]];
    if (!btmp->pid || ((whoincr < 0)
		       ? (btmp->eternal >= mineternal)
		       : (btmp->eternal <= mineternal)))
      continue;
    mineternal = btmp->eternal;
    done++;

    tdif = curr_time - btmp->time;
    tdif /= 60;
    min = tdif % 60;
    hour = tdif / 60;

    msg_status = btmp->xstat ? '*' : SP;

    switch (level)
    {
	      case 4:
       colorize("@Y%s is %s\n", btmp->name, btmp->doing);
       break;

      case 1:
        {
          char work2[60];

          sprintf(work2, "%s%s%s", btmp->remlogin, *btmp->remlogin ? "@@" : "", btmp->remote);
	  colorize("@Y%-19s @M%5d @M%3d @R%2d:%02d%c @C%.37s\n", btmp->name, btmp->pid, btmp->eternal, hour, min, btmp->client ? 'C' : ' ', work2);
        }
	break;
      case 3:
	if (msg_status = ' ')
	  msg_status = ' ';

        if (btmp->elf && !btmp->xstat)
          msg_status = '%';
        if (pos == 3)
          sprintf(work, "%c%s", msg_status, btmp->name);
        else
          sprintf(work, "%c%-19s", msg_status, btmp->name);
	break;
      case 0:
      case 2:
      default:
	if (btmp->elf && !btmp->xstat)
	  msg_status = '%';

            colorize("@Y%-19s %c @R%2d:%02d ", 
              btmp->name, msg_status, hour, min );
	    //if (btmp->sleeptimes > 0)
            //{
            //  colorize("@B");
	    //  colorize ("idle for %2d:%02d\n",
	    //  	btmp->sleeptimes / 60, btmp->sleeptimes % 60);
           // }
	   // else
           // {
              colorize ("@M%-41s\n", btmp->doing);
           // }
		
       //     colorize ("@R%2d:%02d  @C \n", hour, min);
	break;
    }
    if (level == 3)
    {
      if (*work)
        fputs(work, stdout);
      else
        pos--;
      if (++pos == 4)
      {
        pos = 0;
        my_putchar('\n');
      }
      else
        continue;
    }

    if (++l >= rows - 1 && line_more(&l, (done * 100) / (bigbtmp->users + 1)) < 0)
      break;

  }
  if (level == 3 && pos)
    my_putchar('\n');
}



/*
 * is_online (username) 
 *
 * Looks at the btmp file and returns true if the user is currently online. 
 */

struct btmp *
is_online(struct btmp *btmp, struct user *user, char *name)
{
register int i;

  if (user)
  {
    if ((i = user->btmpindex) >= 0)
      if (bigbtmp->btmp[i].pid && bigbtmp->btmp[i].usernum == user->usernum)
      {
        if (btmp)
          *btmp = bigbtmp->btmp[i];
        return(&bigbtmp->btmp[i]);
      }
      else
      {
        locks(SEM_USER);
        if (user->btmpindex >= 0 && (!bigbtmp->btmp[i].pid || bigbtmp->btmp[i].usernum != user->usernum))
          user->btmpindex = -1;
        unlocks(SEM_USER);
      }
    return(NULL);
  }

  for (i = 0; i < MAXUSERS; i++)
    if (bigbtmp->btmp[i].pid && !strcmp(bigbtmp->btmp[i].name, name))
    {
      if (btmp)
        *btmp = bigbtmp->btmp[i];
      return(&bigbtmp->btmp[i]);
    }
  return(NULL);
}




/*
 * Print out the profile of the user listed. 
 *
 * If the name is blank, do yourself. If the user is an aide show everything.
 */

int
profile(char   *name, struct user *tuser, int flags)
{
struct user *tmpuser;
struct btmp userstat;
int online;
int showanon;

  if (!name)
    tmpuser = tuser;
  else if (*name)
  {
    if (!(tmpuser = getuser(name)) || tmpuser->f_invisible && flags != PROF_ALL)
    {
      if (tmpuser)
	freeuser(tmpuser);
      return(-1);
    }
  }
  else
    tmpuser = ouruser;

  if (rows != 32000)
    colorize("@C\n");
  my_printf("%s", tmpuser->name);

  if (tmpuser->f_prog && flags == PROF_ALL)
    colorize("@R *Programmer*");
  if (tmpuser->f_aide)
    colorize("@R *Sysop*");

  if (tmpuser->f_elf && flags == PROF_ALL)
    colorize("@W %%guide%%");

  if (tmpuser->f_twit)
    colorize("@W -TWIT-");
  if (tmpuser->f_newbie)
    colorize("@R (new)");
  if (flags == PROF_ALL)
  {
    if (tmpuser->f_deleted)
      colorize("@R (deleted)");
    if (tmpuser->f_inactive)
      colorize("@R (inactive)");
    if (tmpuser->f_badinfo)
      colorize("@R (bad info)");
    if (tmpuser->f_duplicate)
      colorize("@R (duplicate)");
    if (tmpuser->f_trouble)
      colorize("@R (trouble?)");
  }
  colorize("@Y\n");
      if (*tmpuser->vanityflag)
        colorize ("@G%s\n", tmpuser->vanityflag);

  showanon = (flags == PROF_ALL || flags == PROF_SELF);
  if (showanon || !tmpuser->an_all)
  {
    if (*tmpuser->real_name && (showanon || (!tmpuser->an_all && !tmpuser->an_name)))
    {
      if (tmpuser->an_all || tmpuser->an_name)
	colorize("@WHIDDEN> @Y");
      my_printf("%s\n", tmpuser->real_name);
    }
    if (*tmpuser->addr1 && (showanon || (!tmpuser->an_all && !tmpuser->an_addr)))
    {
      if (tmpuser->an_all || tmpuser->an_addr)
	colorize("@WHIDDEN> @Y");
      my_printf("%s\n", tmpuser->addr1);
    }
    if (*tmpuser->addr2)
      my_printf("%s\n", tmpuser->addr2);

    if (*tmpuser->city && (showanon || (!tmpuser->an_all && !tmpuser->an_location)))
    {
      if (tmpuser->an_all || tmpuser->an_location)
	colorize("@WHIDDEN> @Y");
      my_printf("%s, %s  %s\n", tmpuser->city, tmpuser->state, tmpuser->zip);
    }
    if (*tmpuser->phone && (showanon || (!tmpuser->an_all && !tmpuser->an_phone)))
    {
      if (tmpuser->an_all || tmpuser->an_phone)
	colorize("@WHIDDEN> ");
      colorize("@GPhone: @Y");
      my_printf ("%s\n", tmpuser->phone);
    }
    if (*tmpuser->mail && (showanon || (!tmpuser->an_all && !tmpuser->an_mail)))
    {
      if (tmpuser->an_all || tmpuser->an_mail)
	colorize("@WHIDDEN> ");
      colorize("@GEmail: @Y");
      my_printf ("%s\n", tmpuser->mail);
    }
    if (*tmpuser->www && (showanon || (!tmpuser->an_all && !tmpuser->an_www)))
    {
      if (tmpuser->an_all || tmpuser->an_www)
	colorize("@WHIDDEN> ");
      colorize("@GWWW: @Y");
      my_printf ("%s\n", tmpuser->www);
    }
  }

  online = is_online(&userstat, tmpuser, NULL) ? 1 : 0;
  if (tmpuser->time)
  {
    if (online)
      colorize("@RONLINE since:");
    else
      colorize("@GLast on:");
    colorize (" @M%s", formtime (3, tmpuser->time));
    if (online || tmpuser->timeoff < tmpuser->time)
      if (!showanon && tmpuser->an_site)
        my_putchar('\n');
      else
        colorize("\n");
    else
    {
      if (!showanon && tmpuser->an_site)
	colorize (" until %s\n", formtime (4, tmpuser->timeoff));
      else
	colorize (" until %s @Gfrom @M%.38s\n", formtime (4, tmpuser->timeoff), tmpuser->remote);
    }

      colorize("@GTimes called:@M %ld @GMessages posted:@M %ld @GX messages sent:@M %ld @GUser# @M%ld@G\n", tmpuser->timescalled, tmpuser->posted, tmpuser->totalx, tmpuser->usernum);

    if (flags == PROF_ALL)
    {
      if (tmpuser->f_aide)
	colorize("@GYells handled:@M %d @GValidations done:@M %d@G\n", tmpuser->yells, tmpuser->vals);

      colorize ("@Gcreated: @M%s   ", formtime (3, tmpuser->firstcall));

      if (online)
        colorize("@Gpid: @Y%d  ", userstat.pid);
      if (*tmpuser->loginname)
        colorize("@Gloginname: @Y%s", tmpuser->loginname);
      if (online && userstat.client)
        colorize("@W  (client)");
      if (*tmpuser->remote)
	colorize("\n@Ghostname: @Y%s", tmpuser->remote);
      my_putchar('\n');
    }
/*
    else if (flags != PROF_REG)
      colorize("@GUser# @M%ld\n", tmpuser->usernum);
*/

    if (*tmpuser->doing)
	colorize ("@GDoing: @C%-41s", tmpuser->doing);

    /*if (online && userstat.sleeptimes)
        colorize ("@G[idle for @Y%2d:%02d@G]",
          userstat.sleeptimes / 60, userstat.sleeptimes % 60);*/

    /* If we just printed the above, we need a newline */
    if (*tmpuser->doing || (online && userstat.sleeptimes))
      my_printf ("\n");

    if (online && rows != 32000)
      if (userstat.xstat)
        colorize("@R[eXpress messages DISABLED]\n");
      else
        if (userstat.elf)
          colorize("@R[This user can be eXpressed if you need help with the system]\n");
  }

  colorize("@G");

  if (flags == PROF_ALL && *tmpuser->aideinfo)
    my_printf("%s\n", tmpuser->aideinfo);

  if (*tmpuser->desc1)
    my_printf("\n%s\n", tmpuser->desc1);
  if (*tmpuser->desc2)
    my_printf("%s\n", tmpuser->desc2);
  if (*tmpuser->desc3)
    my_printf("%s\n", tmpuser->desc3);
  if (*tmpuser->desc4)
    my_printf("%s\n", tmpuser->desc4);
  if (*tmpuser->desc5)
    my_printf("%s\n", tmpuser->desc5);

  if (!tuser)
    freeuser(tmpuser);
  return(0);
}



char *
mymmap(char *name, int *size, int priv)
{
  register int f = -1;
  register char *p;

  if (name)
    if ((f = open(name, O_RDWR)) < 0)
      return(NULL);
    else if (size && !*size)
      *size = lseek(f, 0L, SEEK_END);

  p = (char *)mmap(0, *size ? *size : 1, PROT_READ | PROT_WRITE, (f == -1 ? MAP_ANONYMOUS : MAP_FILE) /* | MAP_VARIABLE */ | (priv ? MAP_PRIVATE : MAP_SHARED), f, 0);
  if (f >= 0)
    close(f);

  return(!p || p == (char *)-1 ? NULL : p);
}
