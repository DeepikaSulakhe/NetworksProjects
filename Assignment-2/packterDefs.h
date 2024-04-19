#include <stdint.h>

#define PORT 8080
#define LENGTH 10
#define ACCESS_PERM 0XFFF8

//Response Messages
#define PAID 0XFFFB
#define NOTPAID 0XFFF9
#define NOTEXIST 0XFFFA

// Request Packet Structure
struct RequestPacket {
	uint16_t startPacketID;
	uint8_t clientID;
	uint16_t acc_Per;
	uint8_t segment_Num;
	uint8_t length;
	uint8_t technology;
	unsigned int sourceSubscriberNum;
	uint16_t endPacketID;
};


// Response Packet Structure
struct ResponsePacket {
	uint16_t startPacketID;
	uint8_t clientID;
	uint16_t type;
	uint8_t segment_Num;
	uint8_t length;
	uint8_t technology;
	unsigned int sourceSubscriberNum;
	uint16_t endPacketID;
};