/* Routines for character input and output to the BBS */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>

#include "defs.h"
#include "ext.h"

#ifndef BBS
int ansi = 0;
#endif

static int my_cputs (const char *s);
static int my_puts (const char* s);

/* A standard printf -- no color code recognition. */
/* works best if output is not fflushed */
int
my_printf (const char *fmt, ...)
{
    // delegate to my_vsprintf
    va_list ap;
    va_start(ap, fmt);
    char * buf = my_vsprintf(NULL, fmt, ap);
    va_end(ap);

    // output, then discard the buffer
    int r = my_puts (buf);
    free(buf);
    return r;
}

char   *
my_vsprintf (char *prefix, const char *fmt, va_list ap)
{
    // let vasprintf do all the work for us.
    char   *buf = NULL;
    int     n = vasprintf (&buf, fmt, ap);

    if (n < 0 || buf == NULL) {
        return strdup ("");
    }
    if (prefix == NULL) {
        return buf;
    }

    // concat the prefix and buf.
    char   *result = NULL;

    n = asprintf (&result, "%s%s", prefix, buf);
    free (prefix);
    free(buf);
    if (n < 0 || result == NULL) {
        return strdup ("");
    }
    return result;
}

char*
my_sprintf (char* prefix, const char *fmt, ...)
{
    // delegate to my_vsprintf
    va_list ap;
    va_start(ap, fmt);
    char * buf = my_vsprintf(prefix, fmt, ap);
    va_end(ap);
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
        char * emsg = my_sprintf(NULL,
                "FATAL: vsnprintf system or I/O error. "
                "Called at file '%s':%d "
                " errno said: %s",
                file, line, strerror(saved_errno));
        logfatal(emsg);
    }
    else if (n >= len){
        // error: buffer overflow
        char * emsg = my_sprintf(NULL,
                "FATAL: buffer overflow at file '%s':%d",
                file, line);
        logfatal(emsg);
    }
    return n;
}

char* checked_strcat_with_traceinfo (
        const char* file, int line,
        char* dest, size_t max_dest_size, const char* src)
{
    // we need room for both strings and the null terminator.
    size_t src_len  = strlen(src);
    size_t dest_len = strlen(dest);
    size_t need_size = dest_len + src_len + 1;

    if (need_size <= max_dest_size)
        memcpy( dest + dest_len, src, src_len + 1 );

    else {
        // error: buffer overflow
        char * emsg = my_sprintf(NULL,
                "FATAL: buffer overflow at file '%s':%d", file, line);
        logfatal(emsg);
    }

    return dest;
}

static int
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
    // delegate to my_vsprintf
    va_list ap;
    va_start(ap, fmt);
    char * buf = my_vsprintf(NULL, fmt, ap);
    va_end(ap);

    // write it
    int rc = my_cputs (buf);
    free(buf);
    return rc;
}

/* check for color codes and \r\n translation.  Return the number of characters
   (not including color codes) printed. */
static int
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

