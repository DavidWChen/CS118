#include <string>
#include <time.h>
#include <iostream>
using namespace std;


class Packet
{
	public:
	//timeout value = size 4
	//flags = size 1
	int element = 0;
	int synFlag = 0;//for init
	int finFlag = 0;//for close
	int ACK = 0;//for recv
	int request = 0;
	int lastPkt = 0;
	int retransmit;
	//packet info = size 4
	int seq = 0; //sequence number of packet 1st byte
	int wnd = 5120; //window size
	int pktSize = 1024; //size of actual packet, equal to header + data length
	int srcPort = 0;//port number of source
	int dstPort =0;//port number of destination
	int numPkt =0;
	string filename ="";
	//actual packet data
	//may need to cast to char buffer
	string data = "THIS IS THE DATA";
};

string getSubstring(string str, string pos_str1, string pos_str2)
{
	size_t pos1 = str.find(pos_str1);
	// cout << pos1 << '\n';
	size_t pos2 = str.find(pos_str2);
	// cout << pos2 << '\n';
	string rtn_val = str.substr(pos1+pos_str1.length(), pos2-pos1-pos_str1.length());
	//cout << "return " << rtn_val << '\n';
	return rtn_val;
} 

Packet stringToPacket(string str, Packet packet)
{
	//cout << '\n' << '\n' << "STRING: " << str <<'\n';
	packet.element = stoi(getSubstring(str, " element = ", " synFlag = "));
	packet.synFlag = stoi(getSubstring(str, " synFlag = ", " finFlag = "));
	packet.finFlag = stoi(getSubstring(str, " finFlag = ", " ACK = "));
	packet.ACK = stoi(getSubstring(str, " ACK = ", " request = "));
	packet.request = stoi(getSubstring(str, " request = ", " lastPkt = "));
	packet.lastPkt = stoi(getSubstring(str, " lastPkt = ", " retransmit = "));
	packet.retransmit = stoi(getSubstring(str, " retransmit = ", " seq = "));
	packet.seq = stoi(getSubstring(str, " seq = ", " wnd = "));
	packet.wnd = stoi(getSubstring(str, " wnd = ", " pktSize = "));

	packet.pktSize = stoi(getSubstring(str, "pktSize = ", " srcPort = "));
    packet.srcPort = stoi(getSubstring(str, "srcPort = ", " dstPort = "));
    packet.dstPort = stoi(getSubstring(str, "dstPort = ", " numPkt = "));
    packet.numPkt = stoi(getSubstring(str, "numPkt = ", " filename = "));
    packet.filename = getSubstring(str, "filename = ", " data = ");
    size_t pkt_pos = str.find(" data = ");
    //cout << "pkt pos" << pkt_pos << '\n';
    packet.data = str.substr(pkt_pos+8);//may need to cast to char

	return packet;
}

string PacketToHeader(Packet packet)
{
	string header = " element = " + to_string(packet.element)
		+ " synFlag = " + to_string(packet.synFlag) 
		+ " finFlag = " + to_string(packet.finFlag) 
		+ " ACK = " + to_string(packet.ACK) 
		+ " request = " + to_string(packet.request) 
		+ " lastPkt = " + to_string(packet.lastPkt) 
		+ " retransmit = " + to_string(packet.retransmit)
		+ " seq = " + to_string(packet.seq) 
		+ " wnd = " + to_string(packet.wnd)
		+ " pktSize = " + to_string(packet.pktSize)
		+ " srcPort = " + to_string(packet.srcPort)
		+ " dstPort = " + to_string(packet.dstPort) 
		+ " numPkt = " + to_string(packet.numPkt)
		+ " filename = " + (packet.filename);
		//+ " data  = " + "!";
	return header;
}


	













