CC = gcc
INCLUDE_PATH = `echo ~/include`
INCLUDE = -I$(INCLUDE_PATH)
LPATH = ~/lib
CFLAGS = -O3 -Wall -c
OBJS = foreach.o

all:	$(OBJS)

install:	all
	cp foreach.o $(LPATH); cp foreach.h $(INCLUDE_PATH)

foreach.o:	foreach.c foreach.h
	$(CC) $(CFLAGS) $(INCLUDE) foreach.c

test:	foreach.c foreach.h
	$(CC) -Wall -o foreach -DFOREACH_TEST $(INCLUDE) foreach.c

clean:	
	rm -rf *~ *.o foreach
