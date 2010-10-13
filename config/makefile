CC = gcc
INCLUDE_PATH = `echo ~/include`
FOREACH_PATH = ../foreach
INCLUDE = -I$(INCLUDE_PATH) -I$(FOREACH_PATH)
LPATH = ~/lib
LFOREACH = $(FOREACH_PATH)/foreach.o
CFLAGS = -O3 -Wall -c
OBJS = config.o

all:	$(OBJS)

install:	all
	cp config.o $(LPATH); cp config.h $(INCLUDE_PATH)

config.o:	config.c config.h
	$(CC) $(CFLAGS) $(INCLUDE) config.c

test:	config.c test.conf
	$(CC) -O3 -Wall -DCONFIG_TEST $(INCLUDE) -o config config.c $(LFOREACH)

test.conf:	
	echo arg1 = lala >> test.conf; echo parm3   =    4  >> test.conf; echo hello = k >> test.conf

clean:	
	rm -rf *~ *.o config test.conf
