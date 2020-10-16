#include "defs.h"
#include "ext.h"


int
openfiles(void)
{
 size_t size;

  size = sizeof(struct bigbtmp);
  if (!(bigbtmp = (struct bigbtmp *)mymmap(TMPDATA, &size, 0)))
  {
    my_printf ("btmp\n");
    return(-1);
  }

#if 1
  size = MM_FILELEN;
  if (!(msgstart = (unsigned char *)mymmap(MSGMAIN, &size, 0)))
#else
  size = 0;
  /* Want to set msgsize or whatever to size */
  if (!(msgstart = (unsigned char *)mymmap(MSGMAIN, &size, 0)))
#endif
  {
    my_printf ("msgmain\n");
    return(-1);
  }
/*  madvise((void *)msgstart, size, 0); */

  size = sizeof(struct msg);
  if (!(msg = (struct msg *)mymmap(MSGDATA, &size, 0)))
  {
    my_printf ("msgdata\n");
    return(-1);
  }

  size = 0;
  if (!(xmsg = (unsigned char *)mymmap(XMSGDATA, &size, 0)))
  {
    my_printf ("xmsgdata\n");
    return(-1);
  }
/*  madvise((void *)xmsg, size, 0); */


  size = 0;
  if (!(voteinfo = (struct voteinfo *) mymmap (VOTEFILE, &size, 0)))
  {
    my_printf ("voteinfo problem\n");
    return -1;
  }

  if (openuser() < 0)
  {
    my_printf ("openuser\n");
    return(-1);
  }

  return(0);
}
