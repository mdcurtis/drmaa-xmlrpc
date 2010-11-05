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
OBJS = drmaa-xmlrpc.o dxconfig.o dxlog.o dxopqserial.o

LFOREACH = $(FOREACH_PATH)/foreach.o
LCONFIG = $(CONFIG_PATH)/config.o
LPTHREAD = -lpthread
LXMLRPC = `$(XMLRPC_C_CONFIG) abyss-server --ldadd`
# LXMLRPC_CGI = `$(XMLRPC_C_CONFIG) cgi-server --ldadd`
LDRMAA = -ldrmaa

LIBS_ABYSS = $(LFOREACH) $(LCONFIG) $(LPTHREAD) $(LXMLRPC) $(LDRMAA)

all:	$(OBJS) drmaa-xmlrpc drmaa-xmlrpc.conf

install:	all
	cp drmaa-xmlrpc $(BINPATH)/; cp drmaa-xmlrpc.conf $(CONFPATH);

drmaa-xmlrpc.o:	drmaa-xmlrpc.c
	$(CC) $(CFLAGS) $(INCLUDE) drmaa-xmlrpc.c

drmaa-xmlrpc:	$(OBJS)
	$(LL) $(LFLAGS) -o drmaa-xmlrpc $(OBJS) $(LIBS_ABYSS)

dxconfig.o:	dxconfig.c dxconfig.h
	$(CC) $(CFLAGS) $(INLCUDE) dxconfig.c

dxlog.o:	dxlog.c dxlog.h
	$(CC) $(CFLAGS) $(INCLUDE) dxlog.c

dxopqserial.o:	dxopqserial.c dxopqserial.h
	$(CC) $(CFLAGS) $(INCLUDE) dxopqserial.c

drmaa-xmlrpc.conf:	
	echo daemon = 1 > drmaa-xmlrpc.conf; \
	echo port = 41334 >> drmaa-xmlrpc.conf; \
	echo pid_file = /tmp/drmaa-xmlrpc.pid >> drmaa-xmlrpc.conf; \
	echo abyss_log_file_name = /dev/null >> drmaa-xmlrpc.conf; \
	echo log_file = /tmp/drmaa-xmlrpc.log >> drmaa-xmlrpc.conf; \
	echo log_mask = -1 >> drmaa-xmlrpc.conf;

clean:	
	rm -rf *~ *.o drmaa-xmlrpc drmaa-xmlrpc.cgi drmaa-xmlrpc.conf stub-drmaa-xmlrpc
