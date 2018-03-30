CC=g++
CPPFLAGS=-g -Wall -std=c++11
USERID1=904622289
USERID2=704579512
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
	tar -cvzf project2_704579512_9046222289.tar server.cpp client.cpp packet.h *.bin README Makefile *.doc
