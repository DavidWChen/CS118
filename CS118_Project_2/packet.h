#include <string>
using namespace std;

class Packet
{
	public:
	//timeout value = size 4
	float RTO; //4

	//flags = size 1
	bool synFlag = 0;;//for init
	bool finFlag = 0;;//for close
	bool ACK = 0;//for recv
	bool request = 1;
	bool lastPkt =0;
	bool retransmit;
	//packet info = size 4
	int seq = 0; //sequence number of packet 1st byte
	int wnd = 5120; //window size
	int pktSize = 1024; //size of actual packet, equal to header + data length
	int sourcePort;//port number of source
	int destPort;//port number of destination
	
	string filename;
	//actual packet data
	char* data;
};

string getSubstring(string str, string pos_str1, string pos_str2)
{
	size_t pos1 = str.find(pos_str1);
	size_t pos2 = str.find(pos_str2);
	return str.substr(pos1+sizeof(pos_str1)/sizeof(char),pos_str2);
}

Packet stringToPacket(string str, Packet packet)
{
	// size_t pos_RTO = str.find("RTO = ");
	// size_t pos_synFlag = str.find(" synFlag = ");
	// size_t pos_finFlag = str.find(" finFlag = ");
	// size_t pos_ACK = str.find(" ACK = ");
	// size_t pos_request = str.find(" request = ");
	// size_t pos_lastPkt = str.find(" lastPkt = ");
	// size_t pos_seq = str.find(" seq = ");
	// size_t pos_wnd = str.find(" wnd = ");
	// size_t pos_pktSize = str.find(" pktSize = ");
	// size_t pos_sourcePort = str.find(" sourcePort = ");
	// size_t pos_destPort = str.find(" destPort = ");
	// size_t pos_filename = str.find(" filename = ");
	// size_t pos_data = str.find(" data = ");


	
	packet.RTO = float(getSubstring(str, "RTO = ", " synFlag = "));
	packet.RTO = float(getSubstring(str, "RTO = ", " synFlag = "));
	packet.RTO = float(getSubstring(str, "RTO = ", " synFlag = "));
	packet.RTO = float(getSubstring(str, "RTO = ", " synFlag = "));
	packet.RTO = float(getSubstring(str, "RTO = ", " synFlag = "));
	packet.RTO = float(getSubstring(str, "RTO = ", " synFlag = "));
	packet.RTO = float(getSubstring(str, "RTO = ", " synFlag = "));
	packet.RTO = float(getSubstring(str, "RTO = ", " synFlag = "));
	packet.RTO = float(getSubstring(str, "RTO = ", " synFlag = "));
	packet.RTO = float(getSubstring(str, "RTO = ", " synFlag = "));
	packet.RTO = float(getSubstring(str, "RTO = ", " synFlag = "));

	return packet;
}



















