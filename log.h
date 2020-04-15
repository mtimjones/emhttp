/*===========================================================================
 * log.h
 * Log symbolics and function prototypes.
 * M. Tim Jones <mtj@mtjones.com>
 *=========================================================================*/

#ifndef _LOG_H
#define _LOG_H

#define PREFIX_BYTE	0xfa
#define SUFFIX_BYTE	0xf3

#define NORMAL_REQUEST		0x00
#define UNKNOWN_FILE		0x01
#define TEST_ITEM               0x02

void emitByte(unsigned char);
void emitString(char *);
void sendLog(int);

#endif /* _LOG_H */
