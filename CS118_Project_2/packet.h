#include <string>
#include <time.h>
#include <iostream>
using namespace std;


class Packet
{
    public:
    uint32_t element = 0;
    uint8_t synFlag = 0;     //for init
    uint8_t finFlag = 0;     //for close
    uint8_t ackFlag = 0;     //for recv
    uint16_t seq = 0;        //sequence number of packet 1st byte
    uint32_t numPkt = 0;
    uint8_t data [1011] = {0}; 
};

Packet BufferToPacket(uint8_t * buffer)
{
    Packet packet;
    packet.element = uint32_t(int(buffer[0])
        +int(buffer[1])*256
        +int(buffer[2])*256*256
        +int(buffer[3])*256*256*256);

    packet.synFlag = uint8_t(buffer[4]);
    packet.finFlag = uint8_t(buffer[5]);
    packet.ackFlag = uint8_t(buffer[6]);
    packet.seq = uint16_t(int(buffer[7])
        +int(buffer[8])*256);
    cout << packet.seq;

    packet.numPkt = uint32_t(int(buffer[9])
        +int(buffer[10])*256
        +int(buffer[11])*256*256
        +int(buffer[12])*256*256*256);
    memcpy(&packet.data, &buffer[13], 1011);
    return packet;
}

void PacketToBuffer(Packet packet, uint8_t * buffer)
{
    memset(buffer, '\0', 1024);

    memcpy(&buffer[0], &packet.element, 4);
    memcpy(&buffer[4], &packet.synFlag, 1);
    memcpy(&buffer[5], &packet.finFlag, 1);
    memcpy(&buffer[6], &packet.ackFlag, 1);
    memcpy(&buffer[7], &packet.seq, 2);
    memcpy(&buffer[9], &packet.numPkt, 4);
    memcpy(&buffer[13], &packet.data, 1011);

    return;
}
