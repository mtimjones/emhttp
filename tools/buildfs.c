/*============================================================================
 * buildfs.c
 * The build tool that takes a path and extracts the files for inclusion in
 * the static internal file system.  The output is a C source file that can
 * be compiled with the Embedded HTTP Server.
 * M. Tim Jones <mtj@mtjones.com>
 *==========================================================================*/

#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#define BLK_SIZE	256

#define OUTFILE         "filedata.c"

/* File format:
 *
 *  FILE_HEADER           2 bytes
 *  FILE NAME             variable (null terminated)
 *  FILE SIZE             4 bytes
 *  FILE                  variable
 *
 */

FILE *outfile;
const unsigned char file_header[]={0xfa, 0xf3};
int verbose = 0;
static int counter = 0;

/*----------------------------------------------------------------------------
 * searchdir() - Walk through a directory and identify each file.
 *--------------------------------------------------------------------------*/
void searchdir(char *dir)
{
  DIR *dp;
  struct dirent *entry;
  struct stat statbuf;
  struct stat baselinebuf;
  static int dirsize = 0;
  char fullname[BLK_SIZE]={0};

  if (dirsize == 0) {
    dirsize = strlen(dir);
  }

  if ((dp = opendir(dir)) == NULL) {
    return;
  }

  chdir(dir);
  while ((entry = readdir(dp)) != NULL) {

    lstat(entry->d_name, &statbuf);
    if (S_ISDIR(statbuf.st_mode)) {

      if (entry->d_name[0] == '.') continue;
      searchdir(entry->d_name);

    } else {

      getcwd(fullname, BLK_SIZE);
      if (strlen(fullname) > 1) strcat(fullname, "/");
      strcat(fullname, entry->d_name);
      if (verbose) {
        printf("Cached %s, size %d\n", &fullname[dirsize], statbuf.st_size);
      }
      writeFile(fullname, &fullname[dirsize], statbuf.st_size);

    }

  }

  writeFile(NULL, NULL, 0);

  chdir("..");
  closedir(dp);

}


/*----------------------------------------------------------------------------
 * writeHeader() - Write out the file header info.
 *--------------------------------------------------------------------------*/
void writeHeader(void)
{
  fprintf(outfile, "/*\n * filedata.c\n */\n\n");
  fprintf(outfile, "unsigned char filedata[]={");
}


/*----------------------------------------------------------------------------
 * writeByte() - Write out a single byte to the file.
 *--------------------------------------------------------------------------*/
void writeByte(unsigned char byte)
{
  if ((counter++ % 12) == 0) fprintf(outfile, "\n ");
  fprintf(outfile, " 0x%02x,", byte);
}


/*----------------------------------------------------------------------------
 * writeFile() - Main routine to write the source file.
 *--------------------------------------------------------------------------*/
int writeFile(char *inpfname, char *outfname, int size)
{
  unsigned char *filedata;
  FILE *fin;
  int i, ret;
  static int hdr = 0;

  if (!hdr) {
    writeHeader();
    hdr=1;
  }

  if (inpfname != NULL) {
    fin = fopen(inpfname, "rb");

    filedata = (unsigned char *)malloc(size);

    ret = fread(filedata, 1, size, fin);

    if (ret != size) {
      printf("Error reading %s (%d)\n", inpfname, errno);
    } else {

      fprintf(outfile, "\n\n  /* File: %s, size %d */\n", outfname, size);

      counter = 0;

      /* Emit the file header */
      writeByte(file_header[0]);
      writeByte(file_header[1]);
      
      /* Emit the file name */
      for (i = 0 ; i < strlen(outfname)+1 ; i++) {
        writeByte(outfname[i]);
      }

      /* Emit the 4 byte length */
      writeByte(((size >> 24) & 0xff));
      writeByte(((size >> 16) & 0xff));
      writeByte(((size >>  8) & 0xff));
      writeByte((size & 0xff));

      for (i = 0 ; i < size ; i++) {
        writeByte(filedata[i]);
      }

    }

    free(filedata);

  } else {
    writeByte(0); writeByte(0);
  }
}


/*----------------------------------------------------------------------------
 * usage() - Help information.
 *--------------------------------------------------------------------------*/
void usage()
{
  printf("\n    buildfs\n\n");

  printf("\t-h            This help.\n");
  printf("\t-v            Verbose mode.\n");
  printf("\t-s <source>   Specify the root source directory.\n");

  printf("\n");
}


/*----------------------------------------------------------------------------
 * main() - main routeine for the buildfs tool.
 *--------------------------------------------------------------------------*/
main(int argc, char *argv[])
{
  int opt;
  char sourcedir[128]={0};

  while ((opt = getopt(argc, argv, "hvs:")) != -1) {

    switch(opt) {
      case 's':
        strcpy(sourcedir, optarg);
        break;
      case 'v':
        verbose = 1;
        break;
      case 'h':
        usage();
        exit(0);
    }

  }

  if (sourcedir[0] == 0) {
    usage();
    exit(0);
  }

  outfile = fopen(OUTFILE, "wb");  

  searchdir(sourcedir);

  fprintf(outfile, " 0x00, 0x00\n\n};\n");

  fclose(outfile);
}

