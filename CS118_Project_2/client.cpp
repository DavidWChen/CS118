#include <ctype.h>
#include <dirent.h>
#include <iostream>
#include <netdb.h>      // define structures like hostent
#include <netinet/in.h>
#include <signal.h>  /* signal name macros, and the kill() prototype */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // Needed for Strlen
#include <string>
#include <sys/socket.h> // Needed for socket creating and binding
#include <sys/types.h>
#include <time.h>       // To control the timeout mechanism
#include <unistd.h> 
#include "packet.h"
using namespace std;


string filename;
int portno;
struct sockaddr_in serv_addr;
struct hostent *server;  // contains tons of information, including the server's IP address
FILE * file;
int main(int argc, char *argv[])
{
    if (argc < 4) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    server = gethostbyname(argv[1]);  // takes a string like "www.yahoo.com", and returns a struct hostent which contains information, as IP address, address type, the length of the addresses...
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    int fd;
    if ( (fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket failed");
        return 1;
    }
    struct sockaddr_in serveraddr;
    memset( &serveraddr, 0, sizeof(serveraddr) );

    portno = atoi(argv[2]);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(portno);              
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

    if((file = fopen("receive.data", "w+")) == NULL)
    {
        perror("fopen failed");
        return 1;
    }

    filename = argv[3];
    cout << filename;

    Packet packet;

    packet.filename = filename;
    packet.synFlag = 1;
    //packet.sourcePort;
    packet.dstPort = portno;
    char* data[896];

    bool lastPacket = 0;

    while(!lastPacket)
    {
        int length = recvfrom( fd, file, sizeof(file), 0, NULL, 0 );
        if ( length < 0 ) 
        {
            perror( "recvfrom failed" );
            break;
        }

        lastPacket=packet.lastPkt;
    }

    for ( int i = 0; i < 4; i++ ) {
        if (sendto( fd, "hello", 5, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0 ) {
            perror( "sendto failed" );
            break;
        }
        printf( "message sent\n" );
    }

    close( fd );
}



















