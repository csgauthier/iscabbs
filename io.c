/* Routines for character input and output to the BBS */

#include "defs.h"
#include "ext.h"

#ifndef BBS
int ansi = 0;
#endif 



/* A standard printf -- no color code recognition. */
/* works best if output is not fflushed */
my_printf (const char *fmt, ...)
{
    char string [1024];
    va_list ap;

    va_start (ap, fmt);
    (void) vsprintf (string, fmt, ap);
    va_end (ap);
    return my_puts (string);
}


my_puts (s)
char *s;
{
  int count;

  count = 0;
  while (*s) {
    count = count + (my_putchar (*s) != EOF);
    s++;
  }

  return (count);
}



/* A simple printf that recognizes color codes */
colorize (const char *fmt, ...)
{
    char string [1024];
    va_list ap;

    va_start (ap, fmt);
    (void)  vsprintf (string, fmt, ap);
    va_end (ap);
    return my_cputs (string); 
}


/* check for color codes and \r\n translation.  Return the number of characters
   (not including color codes) printed. */
my_cputs (s)
char *s;
{
  int count;

  count = 0;
  while (*s) {

    if (*s == '@') {
      s++;
      if (ansi)
	switch (*s) {
	  case '@':
	    count = count + (my_putchar ('@') != EOF);
	    break;
	  case 'r':
	  case 'R':
	    output ("\033[31m");
	    break;
	  case 'g':
	  case 'G':
	    output ("\033[32m");
	    break;
	  case 'y':
	  case 'Y':
	    output ("\033[33m");
	    break;
	  case 'b':
	  case 'B':
	    output ("\033[34m");
	    break;
	  case 'm':
	  case 'M':
	  case 'p':
	  case 'P':
	    output ("\033[35m");
	    break;
	  case 'c':
	  case 'C':
	    output ("\033[36m");
	    break;
	  case 'w':
	  case 'W':
	    output ("\033[37m");
	    break;
	  case 'd':
	  case 'D':
	    output ("\033[1m\033[33m");
	    break;
	}
      else if (*s == '@') 
	count = count + (my_putchar (*s) != EOF);
    } else
      count = count + (my_putchar (*s) != EOF);
    s++;
  }

  return count;
}


output (s)
char *s;
{
  while (*s) {
    my_putchar (*s);
    s++;
  }
  return 0;
}


my_putchar (c)
int c;
{
#ifdef _SSL
  char newline = '\r';

  if (c == '\n')
    SSL_write (1, &newline, sizeof (newline));
  SSL_write (1, &c, sizeof (c));
  return (c);

#else
  if (c == '\n')
    putchar ('\r');

  return (putchar (c));
#endif
}

my_putc (c, stream)
int c;
FILE *stream;
{
  int i;
  i = putc (c, stream);

  return (i);
}
