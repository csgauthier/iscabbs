#include "defs.h"
#include "ext.h"


int
openfiles(void)
{
 size_t size;

  size = sizeof(struct bigbtmp);
  if (!(bigbtmp = (struct bigbtmp *)mmap_file(TMPDATA, &size)))
  {
    my_printf ("btmp\n");
    return(-1);
  }

#if 1
  size = MM_FILELEN;
  if (!(msgstart = mmap_file(MSGMAIN, &size)))
#else
  size = 0;
  /* Want to set msgsize or whatever to size */
  if (!(msgstart = mmap_file(MSGMAIN, &size)))
#endif
  {
    my_printf ("msgmain\n");
    return(-1);
  }
/*  madvise((void *)msgstart, size, 0); */

  size = sizeof(struct msg);
  if (!(msg = (struct msg *)mmap_file(MSGDATA, &size)))
  {
    my_printf ("msgdata\n");
    return(-1);
  }

  size = 0;
  if (!(xmsg = mmap_file(XMSGDATA, &size)))
  {
    my_printf ("xmsgdata\n");
    return(-1);
  }
/*  madvise((void *)xmsg, size, 0); */


  size = 0;
  if (!(voteinfo = (struct voteinfo *) mmap_file (VOTEFILE, &size)))
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
