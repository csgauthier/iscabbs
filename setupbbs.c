#include <unistd.h>
#include "defs.h"
#include "ext.h"

#define XMSGSIZE 5000000

char *zerobuf;
size_t zerobuflen = 1048576;

static void
tick (void)
{
    write (1, ".", 1);
}

static void
setupbtmp(void)
{
  int fd;
  struct bigbtmp bigbtmp;

  if ((fd = open (TMPDATA, O_RDWR | O_CREAT, 0660)) < 0)
  {
    perror ("can't open tmpdata");
    return;
  }

  bzero (&bigbtmp, sizeof (struct bigbtmp));
  write (fd, (char *)&bigbtmp, sizeof (struct bigbtmp));
  close (fd);
  printf ("btmp setup complete\n");
}

static void
setupmsgmain(void)
{
  int fd;
  long i;

  if ((fd = open (MSGMAIN, O_WRONLY | O_CREAT, 0660)) < 0)
  {
    perror ("can't open msgmain");
    return;
  }

  for (i = MM_FILELEN; i > 0;) {
    size_t res;
    res = write (fd, zerobuf,
		 ((zerobuflen) > i ? i : zerobuflen));
    if (res <= 0) {
	perror ("writing msgmain");
	return;
    }
    i -= res;
    tick ();
  }
  close (fd);
  printf ("msgmain done\n");
}


static void
setupmsgdata(void)
{
  int fd;
  struct msg msg;

  if ((fd = open (MSGDATA, O_RDWR | O_CREAT, 0660)) < 0) {
    perror ("can't open msgdata");
    return;
  }

  bzero ((char *) &msg, sizeof (struct msg));
  msg.xmsgsize = XMSGSIZE;
  write (fd, (char *)&msg, sizeof (struct msg));
  close (fd);
  printf ("msg data done\n");
}


static void
setupuserdata(void)
{
  int fd;
  int i;

  if ((fd = open (USERDATA, O_WRONLY | O_CREAT, 0660)) < 0)
  {
    perror ("can't open userdata");
    return;
  }

  i = sizeof (struct userdata) + sizeof (struct user) * MAXTOTALUSERS;
  while (i > 0) {
    size_t res;
    res = write (fd, zerobuf,
		 ((zerobuflen) > i ? i : zerobuflen));
    if (res <= 0) {
	perror ("writing msgmain");
	return;
    }
    tick ();
    i -= res;
  }
  close (fd);

  printf ("userdata done\n");
}


static void
setupxmsgdata(void)
{
  int fd;
  int i;

  if ((fd = open (XMSGDATA, O_WRONLY | O_CREAT, 0660)) < 0)
  {
    perror ("can'topen xmsgdata");
    return;
  }

  i = XMSGSIZE;
  while (i > 0) {
    size_t res;
    res = write (fd, zerobuf,
		 ((zerobuflen) > i ? i : zerobuflen));
    if (res <= 0) {
	perror ("writing msgmain");
	return;
    }
    tick ();
    i -= res;
  }
  close (fd);

  printf ("xmsgdata done\n");
}

static void
setupvoteinfo(void)
{
  struct voteinfo voteinfo;
  int fd;

  bzero (&voteinfo, sizeof (struct voteinfo));
  if ((fd = open (VOTEFILE, O_RDWR | O_CREAT, 0664)) < 0)
  {
    perror ("voteinfo");
    return;
  }

  write (fd, (char*)& voteinfo, sizeof (struct voteinfo));
  close (fd);
  printf ("Voteinfo done\n");
}


int
main(int argc, char *argv[])
{
  if ((zerobuf = calloc (zerobuflen, 1)) == NULL) {
    perror ("Couldn't allocate zero buffer");
    exit (1);
  }
  setupbtmp();
  setupmsgmain();
  setupmsgdata();
  setupuserdata();
  setupxmsgdata();
  setupvoteinfo();
  return 0;
}

