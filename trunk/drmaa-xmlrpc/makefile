CC = gcc
LL = gcc
INCLUDE_PATH = `echo ~/include`
INCLUDE = -I$(INCLUDE_PATH)
XMLRPC_C_CONFIG = /home/levk/bin/xmlrpc-c-config
LPATH = ~/lib
BINPATH = ~/bin
# CGIBINPATH = ~/www/cgi-bin
ETCPATH = ~/etc
CONFPATH = ~/opt
CFLAGS = -O3 -Wall -c
LFLAGS = -Wall -Wl,`$(XMLRPC_C_CONFIG) --ldflags | awk -F"R" '{ print $$1 "rpath," $$2 }'`
OBJS = drmaa-xmlrpc.o

LFOREACH = $(LPATH)/foreach.o
LCONFIG = $(LPATH)/config.o
LPTHREAD = -lpthread
LXMLRPC = `$(XMLRPC_C_CONFIG) abyss-server --ldadd`
# LXMLRPC_CGI = `$(XMLRPC_C_CONFIG) cgi-server --ldadd`
LDRMAA = -ldrmaa

LIBS_ABYSS = $(LFOREACH) $(LCONFIG) $(LSL_BATCH) $(LBUNDLE) $(LPTHREAD) $(LXMLRPC) $(LUUID) $(LDRMAA)
# LIBS_CGI = $(LFOREACH) $(LCONFIG) $(LSL_BATCH) $(LBUNDLE) $(LXMLRPC_CGI) $(LUUID)

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

drmaa-xmlrpc.conf:	
	echo daemon = 1 > drmaa-xmlrpc.conf; \
	echo port = 41334 >> drmaa-xmlrpc.conf; \
	echo pid_file = drmaa-xmlrpc.pid >> drmaa-xmlrpc.conf; \
	echo abyss_log_file_name = /dev/null >> drmaa-xmlrpc.conf;

clean:	
	rm -rf *~ *.o drmaa-xmlrpc drmaa-xmlrpc.cgi drmaa-xmlrpc.conf
