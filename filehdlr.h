/*============================================================================
 * filehdlr.h 
 * Internal file system structures and function prototypes.
 * M. Tim Jones <mtj@mtjones.com>
 *==========================================================================*/

#ifndef _FILEHDLR_H
#define _FILEHDLR_H

struct fileHdrStruct {
  int  hdrStart;
  int  size;
  int  fileStart;
};

int lookupFilename(char *, struct fileHdrStruct *);

#endif /* _FILEHDLR_H */

