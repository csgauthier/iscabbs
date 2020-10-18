/*
 * term.c - Handles terminal I/O -- maybe eventually be removed.
 */
#include <sys/ioctl.h>
#include "defs.h"
#include "ext.h"

void
termset(void)
{
struct winsize mywin;

  if (tty && !ioctl(1, TIOCGWINSZ, &mywin))
    rows = mywin.ws_row;

  if (rows < 5 || rows > 100)
    rows = 24;

  if (ouruser && ouruser->f_ansi)
	askansi();
}


void
askansi(void)
{
  my_printf ("\nAre you on an ANSI terminal? (Y/N) -> ");
  if (yesno (-1))
  {
    output ("\033[1m\033[32m");		/* Green High Intensity */
    ansi = 1;
  }
  else
  {
    if (ansi != 0)		/* Reset after change */
      output ("\033[37m\033[0m");
    ansi = 0;
  }
}
