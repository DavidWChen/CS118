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
using namespace std;

void ACKed(Packet packet, time_t timers[], bool cwnd[])//Server
{
    timers[packet.element] = 0;
    cwnd[packet.element] = 0;
    cout << "Receiving packet " << packet.ACK<< endl;
}

int portno;


int main(int argc, char *argv[])
{
    FILE* file;
    int fileLen;
    char *buf;
////Init Server/////////////////////////////////////////////////////////////
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

///////HandShake/////////////////////////////////////////////////////////////////////
    char buffer[1024];
    int length = recvfrom(fd, buffer, sizeof(buffer)-1, 0, (struct sockaddr*)&clientaddr, &clientLen); //receive request from client
    string buffString = buffer;
    //cout << getSubstring(buffString, " element = ", " synFlag = ");
    Packet SYN;
    SYN = stringToPacket(buffString,SYN); //set SYN packet to info from client
    cout << "Receiving packet " << SYN.seq << endl; //print ACK to screen

    // string too_send = "Test String Sent";
    // sendto(fd, too_send.c_str(), sizeof(too_send), 0, (struct sockaddr *)&clientaddr, clientLen);
    
    //cout << SYN.filename;
    string requestedFile = SYN.filename;
    //cout << requestedFile << '\n';
    if ((file = fopen(requestedFile.c_str(), "r")) == NULL)
    {
        cout << "404" << '\n';//set file to 404

        file = fopen("404.html", "r");
        requestedFile = "404.html";
    }
    else
        cout << "Not 404" << '\n';         

    //read file into buffer, convert buffer to string called dataString
    fseek(file, 0, SEEK_END);
    fileLen=ftell(file);
    fseek(file, 0, SEEK_SET);

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

    string dataString = buf;
    //cout << strlen(buf) << " This is the file: " << dataString << endl;

    int dataSize = 896;
    int numPackets = ceil(strlen(buf)/double(dataSize)); //may need to use a double
    //cout << "numpackets server side = " << numPackets << endl;

    Packet SYNAck;
    SYNAck.synFlag = 1;
    SYNAck.ACK = 1;
    SYNAck.numPkt = numPackets;
    SYNAck.filename = requestedFile;
    //cout << SYNAck.filename << endl;    
    string to_send = PacketToHeader(SYNAck) + " data = ";
    //cout << to_send << '\n';
    sendto(fd, to_send.c_str(), strlen(to_send.c_str()), 0, (struct sockaddr *)&clientaddr, clientLen);
    cout << "Sending packet 0 5120 SYN" << '\n';



////Make Packets//////////////////////////////////////////////////////////////////////////////////////
    //establish arrays of packets, the windows in question, and timers for timeout sake
    Packet packets[numPackets];
    bool cwnd[numPackets];
    time_t timers[numPackets];

    for (int i = 0; i < numPackets; i++)
    {
        cwnd[i] = 0;
        timers[i] = 0;
    }

    //initialize all the packets
    for (int i = 0; i < numPackets; i++)
    {
        packets[i].element = i;
        packets[i].seq = (dataSize*i+1)%30720; //set all sequence numbers for packets
        size_t begin = packets[i].seq;
        size_t length = dataSize/sizeof(char);
        packets[i].data = dataString.substr(begin-1,length); //read in all the data into the packets
        packets[i].filename = requestedFile;

    }
    for (int i = 0; i < numPackets; i++)
        {
            cout << "ELE: " << packets[i].element << endl;
            cout << "SEQ: " << packets[i].seq << endl;
            cout << "DATA: "<< packets[i].data[0] << endl;
        }
    cout << "Last Seq: " << packets[numPackets-1].seq;




///////
    int index = 0;
    time_t timer;

    while(1)
    {
    //Check packets in transit
        int sndPkt = 0;
        for (int i = 0; i < numPackets; i++)
        {
            if (cwnd[i] == 1)
            {
                sndPkt++;
            }
        }

        //Set cwnd
        while(sndPkt <= 5)
        {     
            cwnd[index] = 1; //check later
            index++;
            sndPkt++;
        }

        //Send Packets
        for(int i = 0; i< numPackets; i++)
        {
            if (cwnd[i] == 1 && timers[i] == 0)
            {
                time(&timer);  /* get current time; same as: timer = time(NULL)  */
                // cout << " DATA: " << packets[i].data << endl;
                string to_send = PacketToHeader(packets[i]) + " data = " + packets[i].data;
                // cout << "TO_SEND: " << to_send << endl;
                sendto(fd, to_send.c_str(), strlen(to_send.c_str()), 0, (struct sockaddr *)&clientaddr, clientLen);
                timers[i] = timer;
                //cout << to_send;
                cout << "Sending packet " << packets[i].seq << " " << packets[i].wnd;
                cout << endl;
            }
        }
        
        //Receive ACK 
        //cout <<"I'm Stuck" << endl;
        if((recvfrom( fd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&clientaddr, &clientLen)) >= 0)
        {
            Packet ACK;
            string buffString = buffer;
            //cout << "HI" << endl;
            ACK = stringToPacket(buffString,ACK); //set ACK packet to info from client
            cwnd[ACK.element] = 0;
            timers[ACK.element] = 0;
            // cwnd[++index] = 1;
            sndPkt--;
            cout << "Receiving packet " << ACK.seq << endl; //print ACK to screen
        }

        //Check timeouts
        for(int i = 0; i< numPackets; i++)
        {
            if (cwnd[i] == 1 && timers[i] != 0)
            {
                if ((time(NULL)-timers[i] > 0.5))
                {
                    time(&timer);  /* get current time; same as: timer = time(NULL)  */
                    string to_send = PacketToHeader(packets[i]) + packets[i].data;
                    sendto(fd, to_send.c_str(), strlen(to_send.c_str()), 0, (struct sockaddr *)&clientaddr, clientLen);
                    timers[i] = timer;
                    cout << "Sending packet " << packets[i].seq << " " << packets[i].wnd << " Retransmision";
                    cout << endl;
                }
            }
        }
        // for (int i = 0; i < numPackets; i++)
        // {
        //     cout << cwnd[i] << endl;
        // }
        if ( all_of(cwnd, cwnd+numPackets, [](int i){return i == 0;})  )
        {
           break;
        }
    }
    recvfrom( fd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&clientaddr, &clientLen);

    Packet FIN;
    buffString = buffer;
    FIN = stringToPacket(buffString,FIN);
    //cout << buffString << endl;
    cout << "Receiving packet " << FIN.seq << endl;
    //if( FIN.finFlag == 1)

    Packet FINACK;
    FINACK.finFlag = 1;
    FINACK.ACK = 1;
    FINACK.seq = FIN.seq;
    to_send = PacketToHeader(FINACK) + " data = ";
    sendto(fd, to_send.c_str(), strlen(to_send.c_str()), 0, (struct sockaddr *)&clientaddr, clientLen);
    cout << "Sending packet " << FINACK.seq << " " << FINACK.wnd << " FIN" << endl;

    Packet FINFIN;
    FINFIN.finFlag = 1;
    FINFIN.ACK = 1;
    FINFIN.seq = FIN.seq;
    to_send = PacketToHeader(FINFIN) + " data = ";
    sendto(fd, to_send.c_str(), strlen(to_send.c_str()), 0, (struct sockaddr *)&clientaddr, clientLen);
    cout << "Sending packet " << FINFIN.seq << " " << FINFIN.wnd << " FIN" << endl;

    recvfrom( fd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&clientaddr, &clientLen);
    buffString = buffer;
    FIN = stringToPacket(buffString,FIN);
    cout << "Receiving packet " << FIN.seq << endl;
    //if(FIN)
    //cout << "AT END" << endl;
    close( fd );
}