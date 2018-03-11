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
#include <fstream>
using namespace std;

string filename;
int portno;
struct sockaddr_in serv_addr;
struct hostent *server;  // contains tons of information, including the server's IP address
int numPacketsRecvd = 0;
int numPackets = 0;

string a = "received.data";
ofstream received(a.c_str());

int main(int argc, char *argv[])
{   
    if (argc < 4) {
     fprintf(stderr,"usage %s hostname port\n", argv[0]);
     exit(1);
    }

//create socket
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
    printf("got here\n" );
    struct sockaddr_in serveraddr;
    memset( &serveraddr, 0, sizeof(serveraddr) );

    portno = atoi(argv[2]);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(portno); 
    socklen_t serverLen = sizeof(serveraddr);             
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

    filename = argv[3];
    cout << filename;

    Packet requestPkt;

    requestPkt.filename = filename;
    requestPkt.synFlag = 1;
    requestPkt.request = 1;
    //packet.sourcePort;
    requestPkt.dstPort = portno;
    //char* data[896];

    string requestMessage = PacketToHeader(requestPkt); //create string of request packet
    requestMessage = requestMessage + " data = ";
    cout << getSubstring(requestMessage, "filename = ", " data = ") << '\n';

    while (sendto(fd, requestMessage.c_str(), strlen(requestMessage.c_str()), 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0 ) //should we loop it so it keeps trying?
    {
        perror( "send request failed" );
    }
    //add intial timeout

    cout << "Sending Packet SYN" << endl; //syn output
    cerr << "filename is " << requestPkt.filename << endl; //debugging purposes

    char buf[1024];
    recvfrom( fd, buf, sizeof(buf), 0, (struct sockaddr*)&serveraddr, &serverLen); //receive the synack
    string synAckstr = string(buf);
    cout << "Receiving ACK" << '\n';
    //synack packet
    Packet synAck;
    synAck = stringToPacket(synAckstr, synAck);
    numPackets = synAck.numPkt;

    Packet arrayOfPackets[numPackets]; //all packets coming through the pipe
    Packet ackPackets[numPackets];
    bool * arrayOfRecvdPackets = new bool[numPackets];
    fill(arrayOfRecvdPackets, arrayOfRecvdPackets + numPackets, 0); //1 for every packet received

    for (int i = 0; i < numPackets; i++)
    {
        ackPackets[i].element = i;
        ackPackets[i].ACK = 1;
    }

    while (numPacketsRecvd != numPackets)
    {
        char buff[1024];
        recvfrom(fd, buff, sizeof(buff), 0, NULL, 0);
        string incomingMessage = string(buff);
        Packet temp;
        temp = stringToPacket(incomingMessage, temp);
        cout << "Receiving packet " << temp.seq << endl;
        arrayOfPackets[temp.element] = temp;
        received << temp.data;
        if (arrayOfRecvdPackets[temp.element] == 1)
        {
            //ack got dropped
            ackPackets[temp.element].seq = temp.seq;
            cout << "Sending packet " << temp.seq << " Retransmission" << endl;
            string ack = PacketToHeader(ackPackets[temp.element]);
            sendto(fd, ack.c_str(), strlen(ack.c_str()), 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr)); 
        }
        else if (arrayOfRecvdPackets[temp.element] == 0)
        {
            //send ack
            arrayOfRecvdPackets[temp.element] = 1;
            ackPackets[temp.element].seq = temp.seq;
            cout << "Sending packet " << temp.seq << endl;
            string ack = PacketToHeader(ackPackets[temp.element]);
            sendto(fd, ack.c_str(), strlen(ack.c_str()), 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr)); 
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

    Packet FIN;
    FIN.finFlag = 1;
    string fin = PacketToHeader(FIN);

    int lastSeq = arrayOfPackets[numPackets].seq + (arrayOfPackets[numPackets].data).size();
    cout << "Sending packet " << lastSeq << " FIN" << endl;
    sendto(fd, fin.c_str(), strlen(fin.c_str()), 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr)); //send FIN

    //fs = from server
    char finackfs[1024];
    recvfrom(fd, finackfs, sizeof(finackfs), 0, NULL, 0);
    string finackfsstr = string(finackfs);
    Packet FinAckfs;
    FinAckfs = stringToPacket(finackfsstr, FinAckfs);
    cout << "Receiving packet " << FinAckfs.seq << endl;
    if (FinAckfs.finFlag == 1 && FinAckfs.ACK == 1) //what if its not
    {
        char finfs[1024];
        recvfrom(fd, finfs, sizeof(finfs), 0, NULL, 0);
        string finfsstr = string(finfs);
        Packet Finfs;
        Finfs = stringToPacket(finfsstr, Finfs);
        cout << "Receiving packet " << FinAckfs.seq << endl;

        if (Finfs.finFlag == 1)
        {
            Packet FINACK;
            FINACK.finFlag = 1;
            FINACK.ACK = 1;
            string finack = PacketToHeader(FINACK);
            sendto(fd, finack.c_str(), strlen(finack.c_str()), 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr)); //send FINACL
            usleep(1000000);
            close(fd);
            return 0;
        }

    }












    /*bool lastPacket = 0;

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

    close( fd );*/
}