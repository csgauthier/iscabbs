/* Routines for character input and output to the BBS */

#include <stdio.h>
#include <stdarg.h>

#include "defs.h"
#include "ext.h"

#ifndef BBS
int ansi = 0;
#endif

/* A standard printf -- no color code recognition. */
/* works best if output is not fflushed */
int
my_printf (const char *fmt, ...)
{
    va_list ap;

    // call once to get buffer size
    va_start (ap, fmt);
    int n = vsnprintf (NULL, 0, fmt, ap);
    va_end (ap);
    if (n <= 0)
        return 0; // TODO: handle err.

    // alloc buffer and call again
    char * buf = (char*) calloc(n+1, sizeof(char));
    va_start (ap, fmt);
    vsnprintf (buf, n+1, fmt, ap);
    va_end (ap);
    // TODO: handle err

    int r = my_puts (buf);
    free(buf);
    return r;
}

char*
my_sprintf (const char *fmt, ...)
{
    va_list ap;

    // call once to get buffer size
    va_start (ap, fmt);
    int n = vsnprintf (NULL, 0, fmt, ap);
    va_end (ap);
    if (n <= 0)
        return calloc(1,sizeof(char)); // TODO: handle err.

    // alloc buffer and call again
    char * buf = (char*) calloc(n+1, sizeof(char));
    va_start (ap, fmt);
    vsnprintf (buf, n+1, fmt, ap);
    va_end (ap);
    // TODO: handle err
    return buf;
}

int checked_snprintf_with_traceinfo (
        const char* file, int line,
        char* out, size_t len, const char *fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
    errno = 0;
    const int n = vsnprintf (out, len, fmt, ap);
    const int saved_errno = errno;
    va_end (ap);

    if (n < 0){
        // error: I/O or system error.
        char * emsg = my_sprintf(
                "FATAL: vsnprintf system or I/O error. "
                "Called at file '%s':%d "
                " errno said: %s",
                file, line, strerror(saved_errno));
        logfatal(emsg);
    }
    else if (n >= len){
        // error: buffer overflow
        char * emsg = my_sprintf(
                "FATAL: buffer overflow at file '%s':%d",
                file, line);
        logfatal(emsg);
    }
    return n;
}

int
my_puts (const char* s)
{
  int count = 0;

  while (*s) {
    count += (my_putchar (*s) != EOF);
    ++s;
  }

  return count;
}



/* A simple printf that recognizes color codes */
int
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
int
my_cputs (const char *s)
{
  int count = 0;

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
	count += (my_putchar (*s) != EOF);
    } else
      count += (my_putchar (*s) != EOF);
    ++s;
  }

  return count;
}


int
output (const char *s)
{
  while (*s) {
    my_putchar (*s);
    ++s;
  }
  return 0;
}


int
my_putchar (int c)
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

int
my_putc (int c, FILE* stream)
{
    return putc (c, stream);
}
