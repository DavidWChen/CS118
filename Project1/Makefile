CC=gcc
CPPFLAGS=-g -Wall
USERID=904622289
CLASSES=

default: clean all
	
all: server 

server: $(CLASSES)
	$(CC) -o $@ $^ $(CPPFLAGS) $@.c

clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM server client *.tar.gz *.gif *.jpeg *.jpg

dist: tarball

tarball: clean
	tar -cvzf /tmp/$(USERID).tar.gz --exclude=./.vagrant . && mv /tmp/$(USERID).tar.gz .
