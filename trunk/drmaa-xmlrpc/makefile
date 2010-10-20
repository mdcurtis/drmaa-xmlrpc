CC = gcc
LL = gcc
INCLUDE_PATH = `echo ~/include`
FOREACH_PATH = ../foreach
CONFIG_PATH = ../config
INCLUDE = -I$(INCLUDE_PATH) -I$(FOREACH_PATH) -I$(CONFIG_PATH)
XMLRPC_C_CONFIG = xmlrpc-c-config # /path/to/your/xmlrpc-c-config
BINPATH = ~/bin
# CGIBINPATH = ~/www/cgi-bin
CONFPATH = ~/etc
CFLAGS = -O3 -Wall -c `$(XMLRPC_C_CONFIG) --cflags`
LFLAGS = -Wall -Wl,`$(XMLRPC_C_CONFIG) --ldflags | awk -F"R" '{ print $$1 "rpath," $$2 }'`
OBJS = drmaa-xmlrpc.o

LFOREACH = $(FOREACH_PATH)/foreach.o
LCONFIG = $(CONFIG_PATH)/config.o
LPTHREAD = -lpthread
LXMLRPC = `$(XMLRPC_C_CONFIG) abyss-server --ldadd`
LXMLRPC_CGI = `$(XMLRPC_C_CONFIG) cgi-server --ldadd`
LDRMAA = -ldrmaa

LIBS_ABYSS = $(LFOREACH) $(LCONFIG) $(LPTHREAD) $(LXMLRPC) $(LDRMAA)
LIBS_CGI = $(LFOREACH) $(LCONFIG) $(LXMLRPC_CGI) $(LDRMAA)

all:	$(OBJS) drmaa-xmlrpc drmaa-xmlrpc.conf

install:	all
	cp drmaa-xmlrpc $(BINPATH)/; cp drmaa-xmlrpc.conf $(CONFPATH);

drmaa-xmlrpc.o:	drmaa-xmlrpc.c
	$(CC) $(CFLAGS) $(INCLUDE) drmaa-xmlrpc.c

drmaa-xmlrpc.cgi.o:	drmaa-xmlrpc.c
	$(CC) $(CFLAGS) $(INCLUDE) -DDRMAA_XMLRPC_CGI -o drmaa-xmlrpc.cgi.o drmaa-xmlrpc.c

drmaa-xmlrpc:	drmaa-xmlrpc.o
	$(LL) $(LFLAGS) -o drmaa-xmlrpc drmaa-xmlrpc.o $(LIBS_ABYSS)

drmaa-xmlrpc.cgi:	drmaa-xmlrpc.cgi.o
	$(LL) $(LFLAGS) -o drmaa-xmlrpc.cgi drmaa-xmlrpc.cgi.o $(LIBS_CGI)

stub-drmaa.o:	stub-drmaa.c
	$(CC) $(CFLAGS) $(INCLUDE) stub-drmaa.c

stub-drmaa-xmlrpc:	drmaa-xmlrpc.o stub-drmaa.o
	$(LL) $(LFLAGS) -o stub-drmaa-xmlrpc drmaa-xmlrpc.o $(LFOREACH) $(LCONFIG) $(LPTHREAD) $(LXMLRPC) stub-drmaa.o

drmaa-xmlrpc.conf:	
	echo daemon = 1 > drmaa-xmlrpc.conf; \
	echo port = 41334 >> drmaa-xmlrpc.conf; \
	echo pid_file = drmaa-xmlrpc.pid >> drmaa-xmlrpc.conf; \
	echo abyss_log_file_name = /dev/null >> drmaa-xmlrpc.conf; \
	echo log_file = /tmp/drmaa-xmlrpc.log >> drmaa-xmlrpc.conf; \
	echo log_mask = -1 >> drmaa-xmlrpc.conf;

clean:	
	rm -rf *~ *.o drmaa-xmlrpc drmaa-xmlrpc.cgi drmaa-xmlrpc.conf stub-drmaa-xmlrpc
