#define MAXTOTALUSERS 5000


/*
 * Location info for user data, indexed by ->name and ->num.
 */
struct userlink
{
  int free;		/* Index to next free page/link */
  long usernum;		/* User number */
  char name[MAXALIAS+1];/* User name */
};



/*
 * Main user data structure, user data pages follow immediately after.
 */
struct userdata
{
  int gen;		/* Generation number (see finduser()) */
  int totalusers[2];	/* Total users, used as boundary in ->name and ->num */
  int free[2];		/* Index to next free page/link */
  int retries;		/* Stats: times gen changed in finduser() */
  int unused[1017];
  int which;		/* Which of the mirrored values is currently in use */

  int name[2][MAXTOTALUSERS];
  int num[2][MAXTOTALUSERS];
  struct userlink link[MAXTOTALUSERS];
};


struct voteinfo
{
  struct vote
  {
    char msg[1024];                     /* Message to display */
    char answer[MAXTOTALUSERS];         /* Vote results */
    char inuse;                         /* in use */
  } vote[MAXVOTES];
};
