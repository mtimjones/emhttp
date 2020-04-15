/*============================================================================
 * dyncntnt.h
 * Dynamic content funciton prototypes.
 * M. Tim Jones <mtj@mtjones.com>
 *==========================================================================*/

#ifndef _DYNCNTNT_H
#define _DYNCNTNT_H

int addDynamicContent(char *name, char *(*function)());
void getDynamicContent(char *name, char *content);

#endif /* _DYNCNTNT_H */
