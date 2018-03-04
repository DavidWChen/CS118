CC=g++
CPPFLAGS=-g -Wall -std=c++11
USERID=904622289
CLASSES=

default: clean all
	
all: server client

server: $(CLASSES)
	$(CC) -o $@ $^ $(CPPFLAGS) $@.cpp

client: $(CLASSES)
	$(CC) -o $@ $^ $(CPPFLAGS) $@.cpp

clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM server client *.tar.gz *.gif *.jpeg *.jpg

dist: tarball

tarball: clean
	tar -cvzf /tmp/$(USERID).tar.gz --exclude=./.vagrant . && mv /tmp/$(USERID).tar.gz .
