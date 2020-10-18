/*
 * start up SSL and what not 
 *
 */

#include "defs.h"
#include "ext.h"

void initssl(void)
{
  SSL_CTX *ctx;
  SSL_METHOD *meth;

  SSL_load_error_strings();
  SSLeay_add_ssl_algorithms();
  meth = SSLv23_server_method();
  if (!(ctx = SSL_CTX_new (meth)))
  {
    exit (0);
  }
  
}
