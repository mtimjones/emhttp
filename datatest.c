/*============================================================================
 * datatest.c 
 * Dynamic content test entry
 * M. Tim Jones <mtj@mtjones.com>
 *==========================================================================*/

#include <stdio.h>
#include "dyncntnt.h"

static char dataline[80];

/*----------------------------------------------------------------------------
 * myvarfunc() - Our function to emit dynamic content for "myvar".
 *--------------------------------------------------------------------------*/
char *myvarfunc()
{
  static int myvar = 0;

  myvar++;

  sprintf(dataline, "%d", myvar);

  return(dataline);
}


/*----------------------------------------------------------------------------
 * datatestInit() - Add our entry to the dynamic content module.
 *--------------------------------------------------------------------------*/
void datatestInit()
{
  addDynamicContent("myvar", &myvarfunc);
}

