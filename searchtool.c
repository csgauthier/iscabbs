
/* The user-admin searchtool
 *
 * This can be easily redone to search more fields, but until the need 
 * arrives, we'll do searches for: username, real name, phone, state, 
 * city, email, www, hostname
 */
 
#include "defs.h"
#include "ext.h"

searchtool ()
{
  register struct user *user;
  char string[75];
  register int i;
  int count = 0;
  int found;

  my_printf ("Userfile search\r\n");
  my_printf ("Search the entire userbase for (case-insensitive) -> ");
  get_string ("", sizeof (string), string, -1);
  if (!*string)
    return;

  my_printf ("\n");

  for (i = 1; i < MAXTOTALUSERS; i++)
  {

    /* Read in the userfile */
    if ((user = finduser (0, i, 0)) == NULL)
      continue;    

    if (mystrstr (user->name, string)
      || mystrstr (user->remote, string)
      || mystrstr (user->real_name, string)
      || mystrstr (user->addr1, string)
      || mystrstr (user->addr2, string)
      || mystrstr (user->city, string)
      || mystrstr (user->state, string)
      || mystrstr (user->zip, string)
      || mystrstr (user->phone, string)
      || mystrstr (user->mail, string)
      || mystrstr (user->desc1, string)
      || mystrstr (user->desc2, string)
      || mystrstr (user->desc3, string)
      || mystrstr (user->desc4, string)
      || mystrstr (user->desc5, string)
      || mystrstr (user->www, string)
      || mystrstr (user->doing, string)
      || mystrstr (user->vanityflag, string))
    {  
      my_printf ("  %-21s", user->name);
      if (!(++count % 3))
        my_printf ("\n");
    }
    freeuser (user);
  }

  if (!count)
    my_printf ("\tNo matching users found.\n");
  else if (count % 3)
    my_printf ("\n");
}
