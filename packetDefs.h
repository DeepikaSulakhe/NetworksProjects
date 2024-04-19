#include<stdint.h>

#define PORT_NUMBER 8081
#define TIMEOUT_SECONDS 3 // Set Timeout for ACK timer to 3 sec

//Primitives
#define START_PACKET_ID 0XFFFF 
#define END_PACKET_ID 0XFFFF 
#define CLIENT_ID 0XFF 

//Packet Types
#define DATA_PACKET_TYPE 0XFFF1 
#define ACK_PACKET_TYPE 0XFFF2 
#define REJECT_PACKET_TYPE 0XFFF3 

//Reject sub codes
// Out of Sequence - REJECT sub codes
#define OUT_OF_SEQUENCE_CODE 0XFFF4 
// Length Mismatch - REJECT sub codes
#define LENGTH_MISMATCH_CODE 0XFFF5 
// End of Packet missing - REJECT sub codes
#define END_MISSING_CODE 0XFFF6 
 // Duplicate Packet - REJECT sub codes
#define DUPLICATE_PACKET_CODE 0XFFF7

struct Datapacket {
	uint16_t startPacketID;
	uint8_t clientID;
	uint16_t type;
	uint8_t segment_number;
	uint8_t length;
	char payload[255];
	uint16_t endpacketID;
};
struct Ackpacket {
	uint16_t startPacketID;
	uint8_t clientID;
	uint16_t type;
	uint8_t segment_number;
	uint16_t endpacketID;
};
struct Rejectpacket {
	uint16_t startPacketID;
	uint8_t clientID;
	uint16_t type;
	uint16_t subcode;
	uint8_t segment_number;
	uint16_t endpacketID;
};