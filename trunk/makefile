DIRS = foreach config drmaa-xmlrpc

all:	
	-for d in $(DIRS); do (cd $$d; $(MAKE) $(MFLAGS)); done

install:	
	-for d in $(DIRS); do (cd $$d; $(MAKE) $(MFLAGS) install); done

clean:	
	-for d in $(DIRS); do (cd $$d; $(MAKE) $(MFLAGS) clean); done; rm -rf *~