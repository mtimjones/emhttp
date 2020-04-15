/*============================================================================
 * dyncntnt.c
 * Dynamic content utilities
 * M. Tim Jones <mtj@mtjones.com>
 *==========================================================================*/

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include "filehdlr.h"

static int init = 0;

#define MAX_DYNAMIC_CONTENT	20

struct dynamicContentStructure {
  char variableName[80];
  char *(*pfunc)();
} dynamicContent[MAX_DYNAMIC_CONTENT];


/*----------------------------------------------------------------------------
 * defaultFunction() - Default to fill unused dynamic content rows
 *--------------------------------------------------------------------------*/
static char *defaultFunction()
{
  return("???noinit");
}


/*----------------------------------------------------------------------------
 * initContent() - Initialize the dynamic content array       
 *--------------------------------------------------------------------------*/
void initContent()
{
  int i;

  for (i = 0 ; i < MAX_DYNAMIC_CONTENT ; i++) {
    dynamicContent[i].variableName[0] = 0;
    dynamicContent[i].pfunc = &defaultFunction;
  }
}


/*----------------------------------------------------------------------------
 * addDynamicContent() - Add a new element to the dynamic content array
 *--------------------------------------------------------------------------*/
int addDynamicContent(char *name, char *(*function)()) 
{
  int i;

  if (!init) {
    initContent();
    init=1;
  }

  /* First, ensure that the 'name' does not exist in the current list */
  for (i = 0 ; i < MAX_DYNAMIC_CONTENT ; i++) {
    if (dynamicContent[i].variableName[0] != 0) {
      if (!strcmp(name, dynamicContent[i].variableName)) {
        return(-1);
      }
    }
  }

  /* Next, look for an empty slot */
  for (i = 0 ; i < MAX_DYNAMIC_CONTENT ; i++) {
    if (dynamicContent[i].variableName[0] == 0) {
      strncpy(dynamicContent[i].variableName, name, 80);
      dynamicContent[i].pfunc = function;
    }
  }

  return(0); 
}


/*----------------------------------------------------------------------------
 * getDynamicContent() - Get the dynamic content for a particular entry
 *--------------------------------------------------------------------------*/
void getDynamicContent(char *name, char *content)
{
  int j, i;

  if (!init) {
    initContent();
    init=1;
  }

  for (j = 0 ; j < MAX_DYNAMIC_CONTENT ; j++) {

    /* Search for the name in the list, avoid '>' trailing name... */
    for (i = 0 ; name[i] != '>' ; i++) {
      if (dynamicContent[j].variableName[i] != name[i]) break;
    }

    if (name[i] == '>') {
      /* We've reached the end, so it was a good match */
      strcpy(content, dynamicContent[j].pfunc());
      return;
    }

  }

  strcpy(content, defaultFunction());
  return;
}

