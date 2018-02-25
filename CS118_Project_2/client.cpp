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
using namespace std;


string filename;
int portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;  // contains tons of information, including the server's IP address

int main(int argc, char *argv[])
{
    
    if (argc < 4) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    filename = argv[3];
    cout << filename;
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

    for ( int i = 0; i < 4; i++ ) {
        if (sendto( fd, "hello", 5, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0 ) {
            perror( "sendto failed" );
            break;
        }
        printf( "message sent\n" );
    }

    close( fd );
}