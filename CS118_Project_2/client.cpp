#include <ctype.h>
#include <dirent.h>
#include <iostream>
#include <netdb.h>      // define structures like hostent
#include <netinet/in.h>
#include <signal.h>     /* signal name macros, and the kill() prototype */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // Needed for Strlen
#include <string>
#include <sys/socket.h> // Needed for socket creating and binding
#include <sys/types.h>
#include <time.h>       // To control the timeout mechanism
#include <unistd.h> 
#include "packet.h"
#include <fstream>
#include <sys/time.h>
using namespace std;

int portno;
struct sockaddr_in serv_addr;
struct hostent *server;       // contains tons of information, including the server's IP address

FILE* rfd;
int numPacketsRecvd = 0;
int main(int argc, char *argv[])
{   
    if (argc < 4) {
     fprintf(stderr,"usage %s hostname port\n", argv[0]);
     exit(1);
    }

////Create Socket////
    server = gethostbyname(argv[1]);  // takes a string like "www.yahoo.com", and returns a struct hostent which contains information, as IP address, address type, the length of the addresses...
    if (server == NULL) 
    {
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
    socklen_t serverLen = sizeof(serveraddr);             
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

////Handshake////
    char * filename;
    filename = argv[3];
    Packet SYN;
    SYN.synFlag = 1;
    SYN.element = 9876;
    memcpy(SYN.data, filename, sizeof(filename)/sizeof(char));

    uint8_t sbuffer [1024];

    PacketToBuffer(SYN, sbuffer);
    while (sendto(fd, sbuffer, 1024, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0 ) //should we loop it so it keeps trying?
    {
         perror( "send request failed" );
    }
    cout << "Sending packet SYN" << endl;

    uint8_t rbuffer [1024];
    recvfrom( fd, rbuffer, sizeof(rbuffer), 0, (struct sockaddr*)&serveraddr, &serverLen); //receive the synack
    Packet SYNACK;
    SYNACK = BufferToPacket(rbuffer);
    int numPackets = int(SYNACK.numPkt);
    cout << "Receiving packet " << SYNACK.seq << '\n';

    char* fileLen = (char*)SYNACK.data;
    string FL = string(fileLen);
    int len = stoi(FL);


////Main Client////

    rfd = fopen("received.data", "wb+");

    Packet arrayOfPackets[numPackets]; //all packets coming through the pipe
    Packet ackPackets[numPackets];
    bool arrayOfRecvdPackets[numPackets];
    fill(arrayOfRecvdPackets, arrayOfRecvdPackets + numPackets, 0); //1 for every packet received

    for (int i = 0; i < numPackets; i++)
    {
        ackPackets[i].element = i;
        arrayOfPackets[i].element = '\0';
        ackPackets[i].ackFlag = 1;
    }

    while (numPacketsRecvd != numPackets)
    {
        memset( rbuffer, '\0', sizeof(char)*1024);
        recvfrom(fd, rbuffer, 1024, 0, NULL, 0);
        Packet temp;
        temp = BufferToPacket(rbuffer);

        cout << "Receiving packet " << temp.seq << endl;

        if (arrayOfPackets[temp.element].element == '\0')
        {
            arrayOfPackets[temp.element] = temp;
        }

        if (arrayOfRecvdPackets[temp.element] == 1)
        {
            //Packet Dropped
            ackPackets[temp.element].seq = temp.seq;
            cout << "Sending packet " << temp.seq%30720 << " Retransmission" << endl;

            PacketToBuffer(ackPackets[temp.element], sbuffer);

            sendto(fd, sbuffer, 1024, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr)); 

        }
        else if (arrayOfRecvdPackets[temp.element] == 0)
        {
            //Send Ack
            ackPackets[temp.element].seq = temp.seq%30720;
            PacketToBuffer(ackPackets[temp.element], sbuffer);

            arrayOfRecvdPackets[temp.element] = 1;
            cout << "Sending packet " << temp.seq << endl;
            sendto(fd, sbuffer, 1024, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr)); 

        }

        int counter = 0;
        for (int i = 0; i < numPackets; i++)
        {
            if (arrayOfRecvdPackets[i] == 1)
            {
                counter++;
            }
        }
        if (counter >= numPacketsRecvd)
        {
            numPacketsRecvd = counter; //if the counter
        }
    }
    for (int i = 0; i < numPackets; i++)
    {
    	if( i == numPackets - 1)
    	{
    		int a = len%1011;
    		fwrite(arrayOfPackets[i].data, a, 1, rfd);
    		break;
    	}
    	else
    	{
    		fwrite(arrayOfPackets[i].data, 1011, 1, rfd);
    	}
    }
    
    timeval start_time,  curr_time;
    gettimeofday(&start_time, nullptr);
    gettimeofday(&curr_time, nullptr);

    while ((((curr_time.tv_sec*1000) + (curr_time.tv_usec/1000) - (start_time.tv_sec*1000) + (start_time.tv_usec/1000))) < 5000)
    {
		if ((recvfrom( fd, rbuffer, 1024, MSG_DONTWAIT, NULL, 0)) >= 0)
		{
			Packet temp;
        	temp = BufferToPacket(rbuffer);
        	PacketToBuffer(ackPackets[temp.element], sbuffer);
			cout << "Receiving packet " << temp.seq << endl;
            sendto(fd, sbuffer, 1024, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
            cout << "Sending packet " << temp.seq << "Retransmission" << endl; 
			break;
		}
		gettimeofday(&curr_time, nullptr);
    }
    
    int lastSeq = arrayOfPackets[numPackets-1].seq + len%1011;
    Packet FIN;
    FIN.finFlag = 1;
    FIN.seq = lastSeq%30720;
    PacketToBuffer(FIN, sbuffer);
    cout << "Sending packet " << lastSeq%30720 << " FIN" << endl;
    sendto(fd, sbuffer, 1024, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr)); 

    //fs = from server
    recvfrom(fd, rbuffer, 1024, 0, NULL, 0);
    Packet FinAckfs;
    FinAckfs = BufferToPacket(rbuffer);
    cout << "Receiving packet " << FinAckfs.seq << endl;

    if (FinAckfs.finFlag == 1 && FinAckfs.ackFlag == 1) //what if its not
    {
        recvfrom(fd, rbuffer, 1011, 0, NULL, 0);
        Packet Finfs;
        Finfs = BufferToPacket(rbuffer);
        cout << "Receiving packet " << FinAckfs.seq << endl;

        if (Finfs.finFlag == 1)
        {
            Packet FINACK;
            FINACK.finFlag = 1;
            FINACK.ackFlag = 1;
            FINACK.seq = Finfs.seq;
            PacketToBuffer(FINACK, sbuffer);
            sendto(fd, sbuffer, 1024, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr)); 
            cout << "Sending packet " << FINACK.seq << endl;
            usleep(1000000);
            close(fd);
            return 0;
        }
    }


    close(fd);
}

