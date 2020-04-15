/*============================================================================
 * filehdlr.c
 * Internal filesystem utilities.
 * M. Tim Jones <mtj@mtjones.com>
 *==========================================================================*/

#include "filehdlr.h"
#include "log.h"

extern unsigned char filedata[];

/*----------------------------------------------------------------------------
 * lookupFilename() - Locate a file in the internal file system
 *--------------------------------------------------------------------------*/
int lookupFilename(char *filename, struct fileHdrStruct *filehdr)
{
  int offset = 0;
  int size;
  int i, found;
  int ret;

  while (filedata[offset] != 0) {

    ret = offset;
    found = 1;

    if ((filedata[offset] == 0xfa) && (filedata[offset+1] == 0xf3)) {

      /* Skip the header */
      offset+=2;

      /* Search the file name, but don't stop if not found... */
      for (i = 0 ; filedata[i+offset] != 0 ; i++) {
        if (filename[i] != filedata[i+offset]) {
          found = 0;
        }
      }

      /* Skip the filename and null terminator */
      offset += (i+1);

      /* Get the file size */
      size = (filedata[offset  ] << 24) | (filedata[offset+1] << 16) |
             (filedata[offset+2] <<  8) | (filedata[offset+3]);

      if (found) {
        filehdr->hdrStart = ret;
        filehdr->size = size;
        filehdr->fileStart = (offset+4);
        emitByte(PREFIX_BYTE); emitByte(NORMAL_REQUEST);
        emitString(filename); emitByte(SUFFIX_BYTE);
        return(1);
      }

      offset += (size+4);
    }

  }
  return(0);
}

