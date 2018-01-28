/* A simple server in the internet domain using TCP
   The port number is passed as an argument
   This version runs forever, forking off a separate
   process for each connection
*/
#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>  /* signal name macros, and the kill() prototype */
#include <ctype.h>
#include <dirent.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

char * parse(char* req)
{
    char *file = strtok(req, "/");
    file = strtok(NULL," ");

    printf("%s\n", file);
    return file;
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);  // create socket
    if (sockfd < 0)
        error("ERROR opening socket");
    memset((char *) &serv_addr, 0, sizeof(serv_addr));   // reset memory

    // fill in address info
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd, 5);  // 5 simultaneous connection at most

    //accept connections
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    if (newsockfd < 0)
       error("ERROR on accept");

    int n;
    char buffer[256];

    memset(buffer, 0, 256);  // reset memory

    //read client's message
    n = read(newsockfd, buffer, 255);
    if (n < 0) error("ERROR reading from socket");
    printf("%s\n", buffer);

    //parse(buffer);

    char *file = strtok(buffer, "/");
    file = strtok(NULL," ");
    printf("%s\n", file);
    // printf("%s\n", file);
    int i = 0;
    for (i = 0; i < sizeof(file)/sizeof(char); i++){
      putchar(tolower(file[i]));
    }

    int has_file = 0;

    DIR *d;
    struct dirent *dir;
    d = opendir(".");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strcmp(file, dir->d_name) == 0){
                has_file = 1;
                printf("WE GOT YOUR FILE\n");
                // fileNew = dir->d_name;
            }
            //printf("WE GOT YOUR FILE");
            printf("%s\n", dir->d_name);
        }
        closedir(d);
    }
    
    if(!has_file)
    {
        printf("AFNF");
        //send 4xx notfound
        //close socket
        return -1;
    }

    char * ext = strtok(file, ".");

    char * type = "";
    if (strcmp(ext, "html") == 0 || strcmp(ext, "htm") == 0)
        type = "text/html";
    else if (strcmp(ext, "jpeg") == 0 || strcmp(ext, "jpg") == 0)
        type = "image/jpeg";
    else if (strcmp(ext, "gif") == 0)
        type = "image/gif";
    else
        type = "text/plain";
    printf("getting type");

    //....
    //....



    char *reply = 
    "HTTP/1.1 200 OK\n"
    "Date: Thu, 19 Feb 2009 12:27:04 GMT\n"
    "Server: Apache/2.2.3\n"
    "Last-Modified: Wed, 18 Jun 2003 16:05:58 GMT\n"
    "ETag: \"56d-9989200-1132c580\"\n"
    "Content-Type: text/html\n"
    "Content-Length: 7\n"
    "Accept-Ranges: bytes\n"
    "Connection: close\n"
    "\n"
    "GIF89a˙";

    send(newsockfd, reply, strlen(reply), 0);
    //n = send(newsockfd, &resp, sizeof(file), 0);
    //if (n < 0) error("ERROR writing to socket");

    close(newsockfd);  // close connection
    close(sockfd);

    return 0;
}
