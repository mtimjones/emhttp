/*============================================================================
 * emhttp.c
 * Embedded HTTP Server main.
 * M. Tim Jones <mtj@mtjones.com>
 *==========================================================================*/

#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include "emhttp.h"
#include "filehdlr.h"
#include "dyncntnt.h"
#include "log.h"

const char *notimplemented={"HTTP/1.1 501 Not Implemented\n\n"};
const char *notfound={"HTTP/1.1 404 File not found\n\n"};
const char *success={"HTTP/1.1 200 OK\n"};

char *content_types[]={
	"text/html",
	"application/octet-stream",
	"image/jpeg",
	"image/gif",
	"text/plain"
};


/*---------------------------------------------------------------------------
 * ouch() - signal handler for SIGPIPE (UN*X host only)
 *-------------------------------------------------------------------------*/
void ouch ( int sigNum )
{
  /* 
   * Avoid EPIPE crash... 
   */
}


/*---------------------------------------------------------------------------
 * getFilename() - Parse a filename out of the URL
 *-------------------------------------------------------------------------*/
void getFilename(char *inbuf, char *out, int start)
{
  int i=start, j=0;

  /* 
   * Skip any initial spaces 
   */
  while (inbuf[i] == ' ') i++;

  for ( ; i < strlen(inbuf) ; i++) {
    if (inbuf[i] == ' ') {
      out[j] = 0;
      break;
    }
    out[j++] = inbuf[i];
  }

  if (!strcmp(out, "/")) strcpy(out, "/index.html");
}


/*---------------------------------------------------------------------------
 * determineContentType() - self explanatory
 *-------------------------------------------------------------------------*/
int determineContentType(int fileOffset)
{
  char suffix[20];
  int  i;

  extern unsigned char filedata[];

  fileOffset+=2;

  for ( ; filedata[fileOffset] != 0 ; fileOffset++) {
    if (filedata[fileOffset] == '.') break;
  }

  if (filedata[fileOffset] == 0) return(TEXT_PLAIN);
  else {

    fileOffset++;
    for (i = 0 ; filedata[fileOffset+i] != 0 ; i++) {
      suffix[i] = filedata[fileOffset+i];
    }
    suffix[i] = 0;

    /* 
     * Now that we've go the suffix, determine the content type 
     */
    if (!strncmp(suffix, "html", 4) || !strncmp(suffix, "HTML", 4) ||
	!strncmp(suffix, "htm", 3)  || !strncmp(suffix, "HTM", 3)) {
      return(TEXT_HTML);
    } else if (!strncmp(suffix, "class", 5) || !strncmp(suffix, "CLASS", 5) ||
               !strncmp(suffix, "jar", 3)   ||  !strncmp(suffix, "JAR", 3)) {
      return(OCTET_STREAM);
    } else if (!strncmp(suffix, "jpeg", 4) || !strncmp(suffix, "JPEG", 4) ||
               !strncmp(suffix, "jpg", 4)  || !strncmp(suffix, "JPG", 4)) {
      return(JPEG_IMAGE);
    } else if (!strncmp(suffix, "gif", 3) || !strncmp(suffix, "GIF", 3)) {
      return(GIF_IMAGE);
    } else {
      return(TEXT_PLAIN);
    }
  }
}


/*---------------------------------------------------------------------------
 * returnFileHeader() - Emit the HTTP response message through the socket
 *-------------------------------------------------------------------------*/
void returnFileHeader(int fd, int ct)
{
  char line[80];

  write(fd, success, strlen(success));

  sprintf(line, "Server: emhttp\n"); 
  write(fd, line, strlen(line));

  sprintf(line, "Connection: close\n");
  write(fd, line, strlen(line));

  sprintf(line, "Content-Type: %s\n\n", content_types[ct]);
  write(fd, line, strlen(line));
}


/*---------------------------------------------------------------------------
 * parseAndEmitFile() - Parse the HTML and replace dynamic content tags
 *-------------------------------------------------------------------------*/
void parseAndEmitFile(int fd, struct fileHdrStruct *filehdr)
{
  int i;
  char content[80];

  extern unsigned char filedata[];

  /* Emit the dynamic HTML file replacing the <DATA #> with the appropriate
   * content.
   */

  for (i = 0 ; i < filehdr->size ; i++) {

    if (filedata[filehdr->fileStart+i] == '<') {
      if (!strncmp(&filedata[filehdr->fileStart+i+1], "DATA", 4)) {
        i+= 6;
        getDynamicContent(&filedata[filehdr->fileStart+i], content);

        /*
         * Emit the dynamic content
         */
        write(fd, content, strlen(content));

        for ( ; filedata[filehdr->fileStart+i] != '>' ; i++);
        i++;

      } else {
        write(fd, &filedata[filehdr->fileStart+i], 1);
      }
    } else {
      write(fd, &filedata[filehdr->fileStart+i], 1);
    }

  }
}


/*---------------------------------------------------------------------------
 * returnFile() - High level return message function
 *-------------------------------------------------------------------------*/
void returnFile(int fd, struct fileHdrStruct *filehdr)
{
  int ct;

  extern unsigned char filedata[];

  ct = determineContentType(filehdr->hdrStart);

  returnFileHeader(fd, ct);

  if (ct == TEXT_HTML) {
    parseAndEmitFile(fd, filehdr);
  } else {
    write(fd, &filedata[filehdr->fileStart], filehdr->size);
  }
}


/*---------------------------------------------------------------------------
 * handleConnection() - Parse and handle the current HTTP request message
 *-------------------------------------------------------------------------*/
void handleConnection(int fd)
{
  int len, max, loop;
  char buffer[4096]={0};
  char filename[256]={0};
  int ret;
  struct fileHdrStruct filehdr;

  /*
   * Read in the Request Header 
   */
  max = 0; 
  loop = 1;
  while (loop) {
    len = read(fd, &buffer[max], 255); buffer[max+len] = 0;
    max += len;
    if ((buffer[max-4] == 0x0d) && (buffer[max-3] == 0x0a) &&
        (buffer[max-2] == 0x0d) && (buffer[max-1] == 0x0a)) loop = 0;
  }

  /*
   * Determine request 
   */
  if (!strncmp(buffer, "GET", 3)) {
    getFilename(buffer, filename, 4);
    if (!strncmp(filename, "/log", 4)) {
      sendLog(fd);
    } else {
      ret = lookupFilename(filename, &filehdr);
      if (ret > 0) {
        returnFile(fd, &filehdr);
      } else {
        emitByte(PREFIX_BYTE); emitByte(UNKNOWN_FILE);
        emitString(filename);  emitByte(SUFFIX_BYTE);
        write(fd, notfound, strlen(notfound));
      }
    }
  } else if (!strncmp(buffer, "HEAD", 4)) {
    getFilename(buffer, filename, 5);
    ret = lookupFilename(filename, &filehdr);
    if (ret > 0) {
      returnFile(fd, &filehdr);
    } else {
      emitByte(PREFIX_BYTE); emitByte(UNKNOWN_FILE);
      emitString(filename);  emitByte(SUFFIX_BYTE);
      write(fd, notfound, strlen(notfound));
    }
  } else {
    write(fd, notimplemented, strlen(notimplemented));
  }
}


/*---------------------------------------------------------------------------
 * main() - The embedded HTTP server main
 *-------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
  int listenfd, connfd;
  socklen_t clilen;
  struct sockaddr_in cliaddr, servaddr;

  extern void datatestInit();

  /* Init the dynamic content test func */
  datatestInit();

  (void)signal(SIGPIPE, ouch);

  listenfd = socket(AF_INET, SOCK_STREAM, 0);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(8080);

  bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

  listen(listenfd, 5);

  for ( ; ; ) {

    clilen = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
    if (connfd <= 0) break;

    handleConnection(connfd);
    close(connfd);

  }

  close(listenfd);
  return(0);
}
