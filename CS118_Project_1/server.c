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

int sockfd, newsockfd, portno;

void error(char *msg)
{
    perror(msg);
    exit(1);
}

char *replaceWord(const char *s, const char *oldW, const char *newW)
{
    char *result;
    int i, cnt = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);
 
    for (i = 0; s[i] != '\0'; i++)
    {
        if (strstr(&s[i], oldW) == &s[i])
        {
            cnt++;
            i += oldWlen - 1;
        }
    }

    result = (char *)malloc(i + cnt * (newWlen - oldWlen) + 1);
 
    i = 0;
    while (*s)
    {
        if (strstr(s, oldW) == s)
        {
            strcpy(&result[i], newW);
            i += newWlen;
            s += oldWlen;
        }
        else
            result[i++] = *s++;
    }
    result[i] = '\0';
    return result;
}

int message_handler(char * filename, char * type)
{    
    FILE *file;
    char *buf;
    int fileLen;
    file = fopen(filename, "rb");

    fseek(file, 0, SEEK_END);
    fileLen=ftell(file);
    fseek(file, 0, SEEK_SET);

    buf = (char *) malloc(fileLen+1);
    if (!buf) 
    {
        fprintf(stderr, "Memory error");
        fclose(file);
        return -1;
    }
    fread(buf, fileLen, 1, file);
    fclose(file);

    //Contruct reply
    char header[102400];

    sprintf(header, 
    "HTTP/1.1 200 OK\n"//If this returns, it will always be 200
    "Date: Thu, 19 Feb 2009 12:27:04 GMT\n"//time()
    "Server: Apache/2.2.3\n"
    "Last-Modified: Wed, 18 Jun 2003 16:05:58 GMT\n"
    "ETag: \"56d-9989200-1132c580\"\n"
    "Content-Type: %s\n"
    "Content-Length: %i\n"
    "Accept-Ranges: bytes\n"
    "Connection: close\n"
        "\n", type, fileLen);

    send(newsockfd, header, strlen(header), 0);
    send(newsockfd, buf, fileLen, 0);

    close(newsockfd);  // close connection
    close(sockfd);

    return 0;
}

int main(int argc, char *argv[])
{
    // int sockfd, newsockfd, portno;
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

    //TA CODE ENDS HERE//////////////////////////////////////////////////
    char *filename = strtok(buffer, "/");
    filename = strtok(NULL," ");
    char * to_ext = NULL;

    char * ascii = "%20";
    char * space = " ";
    filename = replaceWord(filename, ascii, space);

    int i = 0;
    for (i = 0; i < sizeof(filename)/sizeof(char); i++){
      putchar(filename[i]);
    }
    to_ext = strdup(filename);

    //Scan directory
    int has_file = 0;
    DIR *d;
    struct dirent *dir;
    d = opendir(".");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strcmp(filename, dir->d_name) == 0){
                has_file = 1;
                printf("WE GOT YOUR FILE\n");
            }
            printf("%s\n", dir->d_name);
        }
        closedir(d);
    }
    
    //Check if the file exists, else 404
    if(!has_file)
    {
       message_handler("404.html", "text/html");
    }

    //Get file extension
    char * ext = strtok(to_ext, ".");
    ext = strtok(NULL, "\0");
    char * type = "";
    if (strcmp(ext, "html") == 0 || strcmp(ext, "htm") == 0)
        type = "text/html";
    else if (strcmp(ext, "jpeg") == 0 || strcmp(ext, "jpg") == 0)
        type = "image/jpeg";
    else if (strcmp(ext, "gif") == 0)
        type = "image/gif";
    else
        type = "application/octet";
    printf("%s\n", ext);
    printf("%s\n", type);

    //Open file
    message_handler(filename, type);
}

