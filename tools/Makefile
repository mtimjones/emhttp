#
# Makefile for the buildfs tool
#

CC = gcc
OBJS =	buildfs.o

all:	buildfs

buildfs: $(OBJS)
	$(CC) -o buildfs $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f buildfs *.o filedata.c
