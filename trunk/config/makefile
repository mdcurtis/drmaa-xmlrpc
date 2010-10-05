CC = gcc
INCLUDE_PATH = `echo ~/include`
INCLUDE = -I$(INCLUDE_PATH)
LPATH = ~/lib
CFLAGS = -O3 -Wall -c
OBJS = config.o

all:	$(OBJS)

install:	all
	cp config.o $(LPATH); cp config.h $(INCLUDE_PATH)

config.o:	config.c config.h
	$(CC) $(CFLAGS) $(INCLUDE) config.c

test:	config.c test.conf
	$(CC) -O3 -Wall -DCONFIG_TEST $(INCLUDE) -o config config.c $(LPATH)/foreach.o

test.conf:	
	echo arg1 = lala >> test.conf; echo parm3   =    4  >> test.conf; echo hello = k >> test.conf

clean:	
	rm -rf *~ *.o config test.conf
