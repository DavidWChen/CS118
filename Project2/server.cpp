#include <stdio.h>      // Default System Calls
#include <stdlib.h>     // Needed for OS X
#include <string.h>     // Needed for Strlen
#include <sys/socket.h> // Needed for socket creating and binding
#include <netinet/in.h> // Needed to use struct sockaddr_in
#include <time.h>       // To control the timeout mechanism
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
#include <math.h>
#include <algorithm>
#include "packet.h"
#include <iostream>
#include <fstream>
#include <sys/time.h>
using namespace std;

int portno;
const int DATASIZE = 1011;


int main(int argc, char *argv[])
{
    FILE* file;
    FILE* ofd;
    int fileLen;
    char * buf;
////Create Socket/////////////////////////////////////////////////////////////
    if (argc < 2) 
    {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    int fd;
    if ( (fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
    {
        perror( "socket failed" );
        return 1;
    }

    struct sockaddr_in serveraddr, clientaddr;
    socklen_t clientLen = sizeof(clientaddr);
    memset( &serveraddr, 0, sizeof(serveraddr) );

    portno= atoi(argv[1]);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(portno);
    serveraddr.sin_addr.s_addr = htonl( INADDR_ANY );

    if ( ::bind(fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0 ) 
    {
        perror( "bind failed" );
        return 1;
    }

    uint8_t rbuffer[1024];
    int length = recvfrom(fd, rbuffer, sizeof(rbuffer)-1, 0, (struct sockaddr*)&clientaddr, &clientLen); //receive request from client
    if (length == 0)
    {
        exit(1);
    }
    Packet SYN;
    SYN = BufferToPacket(rbuffer);
    cout << "Receiving packet " << SYN.seq << endl;

    char * filename;
    filename = (char * ) SYN.data;


    if ((file = fopen(filename, "rb")) == NULL)
    {

        file = fopen("404.html", "rb");
        filename = (char * ) "404.html";
    }

    //read file into buffer, convert buffer to dataString
    fseek(file, 0, SEEK_END);
    fileLen=ftell(file);
    fseek(file, 0, SEEK_SET);

    char leng[1011];
    sprintf(leng, "%d", fileLen);

    //Allocate memory and read file into buffer
    buf = (char *) malloc(fileLen+1);
    if (!buf) 
    {
        fprintf(stderr, "Memory error");
        fclose(file);
        return -1;
    }
    fread(buf, fileLen, 1, file);


    fclose(file);
    
    uint8_t sbuffer[1024];
    uint32_t numPackets = ceil(double(fileLen)/DATASIZE);
    Packet SYNACK;
    SYNACK.synFlag = 1;
    SYNACK.ackFlag = 1;
    SYNACK.numPkt = numPackets;
    memcpy(SYNACK.data, leng, sizeof(filename)/sizeof(char));

    PacketToBuffer(SYNACK, sbuffer);
    sendto(fd, sbuffer, 1024, 0, (struct sockaddr *)&clientaddr, clientLen);
    cout << "Sending packet 0 5120 SYN" << '\n';
    int wndSize = 0;
    if (numPackets >= 5)
    {
        wndSize = 5;
    }
    else
    {
        wndSize = numPackets;
    }

////Make Packets//////////////////////////////////////////////////////////////////////////////////////
    //Packets, Window, Timers arrays
    Packet packets[numPackets];
    uint32_t recvACK[numPackets];
    bool cwnd[numPackets];
    timeval timers[numPackets];

    for (int i = 0; i < numPackets; i++)
    {
        cwnd[i] = 0;
        timers[i].tv_sec = 0;
        timers[i].tv_usec = 0;
        recvACK[i] = numPackets+1;
    }
    ofd = fopen("file.copy", "wb+");


    for (int i = 0; i < numPackets; i++)
    {
        packets[i].element = i;
        packets[i].seq = (DATASIZE*i+1)%30720;      //set all sequence numbers for packets
        if (i != (numPackets-1))
        {
            memcpy(packets[i].data, &buf[i*DATASIZE], DATASIZE);
            fwrite(packets[i].data, 1011, 1, ofd);

        }        
        else
        {
            memcpy(packets[i].data, &buf[i*DATASIZE], fileLen%DATASIZE);
            int a = fileLen%1011;
            fwrite(packets[i].data, a, 1, ofd);

        }
        
    }
/////Main Server Code///////////////////////////// 
    int index = 0;
    timeval send_time, curr_time;
    while(1)
    {
        //Set Congestion Window
        for (int i = 0; i< numPackets; i++)
        {
            cwnd[i] = 0;
        }
        for (int i = index; i < index + 5 && i < numPackets; i++)
        {
            cwnd[i] = 1;
        }

        //////////////////////////////////////////////////////////
        //Send Packets
        for(int i = 0; i< numPackets; i++)
        {
            if (cwnd[i] == 1 && timers[i].tv_sec == 0 && timers[i].tv_usec==0)
            {


                gettimeofday(&send_time, nullptr);
                PacketToBuffer(packets[i], sbuffer);
                sendto(fd, sbuffer, 1024, 0, (struct sockaddr *)&clientaddr, clientLen);
                timers[i] = send_time;

                cout << "Sending packet " << packets[i].seq << " " << "5120" << endl;
            }
        }

        //Receive ACKs
        if((recvfrom( fd, rbuffer, 1024, MSG_DONTWAIT, (struct sockaddr*)&clientaddr, &clientLen)) >= 0)
        {
            Packet ACK;
            ACK = BufferToPacket(rbuffer);
            recvACK[ACK.element] = ACK.element;
            
            //cwnd[ACK.element] = 0;

            timers[ACK.element].tv_sec = 0;
            timers[ACK.element].tv_usec = 0;
            
            cout << "Receiving packet " << ACK.seq << endl; //print ACK to screen
        }
        //Check timeouts
        for(int i = 0; i< numPackets; i++)
        {
            if (cwnd[i] == 1 && timers[i].tv_sec != 0 && timers[i].tv_usec != 0)
            {   
                gettimeofday(&curr_time, nullptr);
                long time_diff = ((curr_time.tv_sec*1000) + (curr_time.tv_usec/1000)) - ((timers[i].tv_sec*1000) + (timers[i].tv_usec/1000));
                if (time_diff >= 500)
                {
                    PacketToBuffer(packets[i], sbuffer);
                    sendto(fd, sbuffer, 1024, 0, (struct sockaddr *)&clientaddr, clientLen);
                    timers[i] = curr_time;
                    cout << "Sending packet " << packets[i].seq << " " << "5120" << " Retransmision" << endl;
                }
            }
        }
//////////////////////////////////////////////////////////////////////////////

        //Set Index
        for (int i = 0; i < numPackets; i++)
            {   
                if (recvACK[i] == numPackets+1)
                {
                    index = i;  
                    break;
                }
                else
                {
                    index = numPackets;  
                }
            }
        //Check exit condition
        int counter = 0;
        for (int i = 0; i < numPackets; i++)
        {
            if (recvACK[i] != numPackets+1)
            {
                counter++;
            }  
        }
        if (counter == numPackets)
        {
            break;
        }
        
    }


    recvfrom( fd, rbuffer, 1024, 0, (struct sockaddr*)&clientaddr, &clientLen);
    Packet FIN;
    FIN = BufferToPacket(rbuffer);
    cout << "Receiving packet " << FIN.seq << endl;

    Packet FINACK;
    FINACK.finFlag = 1;
    FINACK.ackFlag = 1;
    FINACK.seq = FIN.seq;
    PacketToBuffer(FINACK, sbuffer);
    sendto(fd, sbuffer, 1024, 0, (struct sockaddr *)&clientaddr, clientLen);
    cout << "Sending packet " << FINACK.seq << " " << "5120" << " FIN" << endl;

    Packet FINFIN;
    FINFIN.finFlag = 1;
    FINFIN.ackFlag = 0;
    FINFIN.seq = FIN.seq;
    PacketToBuffer(FINFIN, sbuffer);
    sendto(fd, sbuffer, 1024, 0, (struct sockaddr *)&clientaddr, clientLen);
    cout << "Sending packet " << FINFIN.seq << " " << "5120" << " FIN" << endl;

    recvfrom(fd, rbuffer, 1024, 0, (struct sockaddr*)&clientaddr, &clientLen);
    Packet FINFINACK;
    FINFINACK = BufferToPacket(rbuffer);
    cout << "Receiving packet " << FINFINACK.seq << endl;
    close( fd );
}


















