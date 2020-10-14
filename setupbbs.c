#include "defs.h"
#include "ext.h"
#include <err.h>
#include <unistd.h>

#define XMSGSIZE 5000000

char *zerobuf;
size_t zerobuflen = 1048576;

/* Create a file with the given size filled with all zero bytes.
 * Any existing data is overwritten. Exits immediately on failure. */
static void init_file(const char* path, size_t size, mode_t mode)
{
  // truncate the file to zero size, and create it if necessary.
  int fd = open( path, O_RDWR | O_CREAT | O_TRUNC, mode);
  if (fd == -1)
      err( 1, "init_file: failed to open '%s' for writing", path);

  // resize the file, filling it with zeros.
  int rc = ftruncate( fd, size );
  if (rc == -1)
    err( 1, "init_file: ftruncate failed for file '%s'", path);

  close(rc);
}

static void
setupbtmp(void)
{
  init_file( TMPDATA, sizeof(struct bigbtmp), 0660);
  printf ("btmp setup complete.\n");
}

static void
setupmsgmain(void)
{
  init_file( MSGMAIN, MM_FILELEN, 0660);
  printf ("msgmain setup complete.\n");
}

static void
setupmsgdata(void)
{
  // initialize struct msg with non-zero data.
  struct msg * msg = (struct msg*) calloc(1, sizeof(struct msg));
  msg->xmsgsize = XMSGSIZE;

  int fd = open( MSGDATA, O_RDWR | O_CREAT | O_TRUNC, 0660);
  if (fd == -1)
      err( 1, "setupmsgdata: failed to open '%s' for writing", MSGDATA);

  int rc = write( fd, msg, sizeof(struct msg));
  if (rc == -1)
    err( 1, "setupmsgdata: failed to write file '%s'", MSGDATA);

  close(fd);
  free(msg);
  printf ("msg data setup complete\n");
}

static void
setupuserdata(void)
{
  init_file( USERDATA, sizeof (struct userdata) + sizeof (struct user) * MAXTOTALUSERS, 0660);
  printf ("userdata setup complete\n");
}


static void
setupxmsgdata(void)
{
  init_file( XMSGDATA, XMSGSIZE, 0660);
  printf ("xmsgdata setup complete\n");
}

static void
setupvoteinfo(void)
{
  init_file( VOTEFILE, sizeof(struct voteinfo), 0664);
  printf ("voteinfo setup complete\n");
}


int
main(int argc, char *argv[])
{
  setupbtmp();
  setupmsgmain();
  setupmsgdata();
  setupuserdata();
  setupxmsgdata();
  setupvoteinfo();
  return 0;
}

