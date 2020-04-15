#
# Makefile for emhttp
#

CC = gcc

OBJS =	emhttp.o filedata.o filehdlr.o dyncntnt.o log.o

OBJS+= datatest.o

all:	emhttp

emhttp: $(OBJS)
	$(CC) -o emhttp $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -Wall -c $<

clean:
	rm -f emhttp *.o
