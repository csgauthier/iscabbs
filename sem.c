/* Routines for hanling mmap data and semaphores */
#include "defs.h"
#include "ext.h"

static int semid[7] = { -1, -1, -1, -1, -1, -1, -1 };


int msem_init (int *sem, int val)
{
  *sem = val;
  return 1;
}


/* lock a semaphore */
int msem_lock (which, unused)
int *which;
int unused;
{
struct sembuf lock[2];

  if (semid[*which] < 0)
  {
    /* 1000 + which makes for an easy semkey.  Easy to change, too */
    if ((semid[*which] = semget (1000 + *which, 1, IPC_CREAT | 0664)) < 0)
      return -1;
  }

  lock[0].sem_num = 0;
  lock[0].sem_op = 0;
  lock[0].sem_flg = 0;

  lock[1].sem_num = 0;
  lock[1].sem_op = 1;   /* inc by 1 */
  lock[1].sem_flg = SEM_UNDO;


  if (semop (semid[*which], &lock[0], 2) < 0) 
    return -1;

  return 0;
}


/* unlock */
int msem_unlock (which, unused)
int *which;
int unused;
{
struct sembuf unlock;

  unlock.sem_num = 0;
  unlock.sem_op = -1;
  unlock.sem_flg = SEM_UNDO | IPC_NOWAIT;

  if (semop (semid[*which], &unlock, 1) < 0)
    return -1;

  return 0;
}

