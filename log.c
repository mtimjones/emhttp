/*============================================================================
 * log.c
 * Compressed Log utilities.
 * M. Tim Jones <mtj@mtjones.com>
 *==========================================================================*/

#include <unistd.h>
#include "log.h"
#include "emhttp.h"

#define HI_PRIORITY	0x80
#define MED_PRIORITY	0x40
#define LOW_PRIORITY	0x00

#define LOG_TYPE_MASK	0x3f

#define HUNTING_PREFIX_HEADER	0
#define EMIT_STRING             1
#define EMIT_ARGUMENT           2
#define HUNTING_SUFFIX_HEADER   3

struct logConstStruct {
  unsigned char id;
  char          *string;
} logStrings[] = {
  { (NORMAL_REQUEST | LOW_PRIORITY), "Received request for ^.<P>" },
  { (UNKNOWN_FILE   | LOW_PRIORITY), "Request for unknown file ^.<P>" }
};

#define MAX_LOG		1024

static unsigned char log[MAX_LOG];
static int curWrite = 0;


/*----------------------------------------------------------------------------
 * emitByte() - Emit a single byte to the log
 *--------------------------------------------------------------------------*/
void emitByte(unsigned char byte)
{
  log[curWrite++] = byte;
  if (curWrite == MAX_LOG) curWrite = 0;
}


/*----------------------------------------------------------------------------
 * emitString() - Emit a string (with null char) to the log
 *--------------------------------------------------------------------------*/
void emitString(char *string)
{
  int i, len = strlen(string)+1;
  for (i = 0 ; i < len ; i++) emitByte(string[i]);
}


/*----------------------------------------------------------------------------
 * sendLog() - Emit the current log through the connected HTTP client socket.
 *--------------------------------------------------------------------------*/
void sendLog(int fd)
{
  int count = MAX_LOG;
  int i = curWrite;
  int state = HUNTING_PREFIX_HEADER;
  unsigned char curByte;
  char *logLine;

  extern void returnFileHeader(int, int);
  unsigned char getByte(int *, int *);
  unsigned char peekByte(int *);

  const char *HTML_HDR1={"<HTML><HEAD><TITLE>Log</TITLE></HEAD>"};
  const char *HTML_HDR2={"<BODY><H3>"};
  const char *HTML_HDR3={"</H3></BODY></HTML>\n"};

  /* First, make sure we're on a valid header. */
  i = curWrite;
  while (count > 0) {
    if (peekByte(&i) == PREFIX_BYTE) break;
    (void)getByte(&i, &count);
  }

  /* Emit log header info in HTML format */
  returnFileHeader(fd, TEXT_HTML);

  write(fd, HTML_HDR1, strlen(HTML_HDR1));
  write(fd, HTML_HDR2, strlen(HTML_HDR2));

  while (count > 0) {
    if (state == HUNTING_PREFIX_HEADER) {
      if (getByte(&i, &count) == PREFIX_BYTE) {
        curByte = getByte(&i, &count);
        logLine = logStrings[(curByte & LOG_TYPE_MASK)].string;
        state = EMIT_STRING;
        write(fd, "\n", 1);
      } else break;
    } else if (state == EMIT_STRING) {
      curByte = *logLine++;
      if      (curByte ==   0) state = HUNTING_SUFFIX_HEADER;
      else if (curByte == '^') state = EMIT_ARGUMENT;
      else write(fd, &curByte, 1);
    } else if (state == EMIT_ARGUMENT) {
      curByte = getByte(&i, &count);
      if (curByte == 0) state = EMIT_STRING;
      else write(fd, &curByte, 1);
    } else if (state == HUNTING_SUFFIX_HEADER) {
      if (getByte(&i, &count) == SUFFIX_BYTE) state = HUNTING_PREFIX_HEADER;
      else break;
    }
  }
  write(fd, HTML_HDR3, strlen(HTML_HDR3));
}


/*----------------------------------------------------------------------------
 * getByte() - Read a byte from the log (with autoincrement of read index)
 *--------------------------------------------------------------------------*/
unsigned char getByte(int *index, int *count)
{
  (*count)--;
  if (*index == MAX_LOG) *index = 0;
  return(log[(*index)++]);
}


/*----------------------------------------------------------------------------
 * peekByte() - Peek at a byte at the current read index
 *--------------------------------------------------------------------------*/
unsigned char peekByte(int *index)
{
  if (*index == MAX_LOG) return(log[0]);
  else return(log[*index]);
}

