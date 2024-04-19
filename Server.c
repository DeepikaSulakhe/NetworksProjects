

#include<sys/socket.h>	//Main Sockets Header Library
#include<netinet/in.h>	//Internet Address Family Library. IP networking.
#include<stdio.h>
#include<strings.h>
#include<string.h>
#include<stdint.h>
#include<stdlib.h>
#include<unistd.h>
#include "packetDefs.h"
#include<signal.h>

int sockfd;
//Socket Server Shutdown when SIGINT(CTRL+C) is received
void intHandler(int dummy) {
	signal(SIGINT, SIG_IGN);
	printf("\nShutting Down Server\n");
	shutdown(sockfd, SHUT_RDWR);
	exit(0);
}

// function to printout the packet information
void show(struct Datapacket data) {
	printf("Received Packet Details\n");
	printf("Start of Packet ID: %hx\n",data.startPacketID);
	printf("Client ID : %hhx\n",data.clientID);
	printf("Data: %x\n",data.type);
	printf("Segment Number: %d\n",data.segment_number);
	printf("Length: %d\n",data.length);
	printf("Payload: %s\n",data.payload);
	printf("End of Packet ID: %x\n",data.endpacketID);
}
// function to generate the reject packet 
struct Rejectpacket generateRejectpacket(struct Datapacket data) {
	struct Rejectpacket reject;
	reject.startPacketID = data.startPacketID;
	reject.clientID = data.clientID;
	reject.segment_number = data.segment_number;
	reject.type = REJECT_PACKET_TYPE;
	reject.endpacketID = data.endpacketID;
	return reject;
}
// function to generate the ack packet
struct Ackpacket generateAckpacket(struct Datapacket data) {
	struct Ackpacket ack;
	ack.startPacketID = data.startPacketID;
	ack.clientID = data.clientID;
	ack.segment_number = data.segment_number;
	ack.type = ACK_PACKET_TYPE ;
	ack.endpacketID = data.endpacketID;
	return ack;
}

int main(int argc, char**argv)
{
	int n;
	struct sockaddr_in serverAddr;	// For IP networking, we use struct sockaddr_in, which is defined in the header netinet/in.h.
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;
	struct Datapacket data;
	struct Ackpacket  ackData;
	struct Rejectpacket rejectData;

	// a pre defined buffer for all the packets, once the buffer is filled server will not send any response.
	//boolean buffer table
	int buffer[34];
	int j;	
	signal(SIGINT, intHandler);
	for(j=0;j<34;j++) 
	{
		buffer[j] = 0;
	}
	sockfd=socket(AF_INET,SOCK_DGRAM,0); // UDP datagram socket
	int expectedPacket = 1;
	bzero(&serverAddr,sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	//Number Conversions: Functions are used to change formats. Big Endian or Little Endian.
	//HTONL: to convert an IPv4 address in host byte order to the IPv4 address in network byte order.
	serverAddr.sin_addr.s_addr=htonl(INADDR_ANY); // INADDR_ANY= 0.0.0.0
	serverAddr.sin_port=htons(PORT_NUMBER);
	//BIND SYSTEM CALL
	bind(sockfd,(struct sockaddr *)&serverAddr,sizeof(serverAddr));	
	//SOCKADDR: structure taken from <sys/socket.h>
	//SockFD: File Descriptor. Information about incoming packet.
	addr_size = sizeof serverAddr;	//Size of Internet Address.
	printf("Server running\n");

	// start receiving from client 
	while(1) {
		printf("\n");
		// receiving the packet from client
		n = recvfrom(sockfd,&data,sizeof(struct Datapacket),0,(struct sockaddr *)&serverStorage, &addr_size);
		printf("New Packet Arriving........ \n");
		show(data);
		int length = strlen(data.payload);
		//for execution purpose testing
		printf("Expected Packet:%d\n",expectedPacket);  
		if(buffer[data.segment_number] != 0) 
		{
			rejectData = generateRejectpacket(data);
			rejectData.subcode = DUPLICATE_PACKET_CODE;
			sendto(sockfd,&rejectData,sizeof(struct Rejectpacket),0,(struct sockaddr *)&serverStorage,addr_size);
			printf("....  DUPLICATE PACKET RECIEVED .... \n\n");
		}

		else if(length != data.length) 
		{
			rejectData = generateRejectpacket(data);
			rejectData.subcode = LENGTH_MISMATCH_CODE ;
			sendto(sockfd,&rejectData,sizeof(struct Rejectpacket),0,(struct sockaddr *)&serverStorage,addr_size);
			printf("....  LENGTH MISMATCH ERROR .... \n\n");
		}
		else if(data.endpacketID != END_PACKET_ID ) 
		{
			rejectData = generateRejectpacket(data);
			rejectData.subcode = END_MISSING_CODE ;
			sendto(sockfd,&rejectData,sizeof(struct Rejectpacket),0,(struct sockaddr *)&serverStorage,addr_size);
			printf("....  END OF PACKET IDENTIFIER MISSING .... \n\n");
		}
		else if(data.segment_number != expectedPacket && data.segment_number != 10 && data.segment_number != 11) 
		{
			rejectData = generateRejectpacket(data);
			rejectData.subcode = OUT_OF_SEQUENCE_CODE;
			sendto(sockfd,&rejectData,sizeof(struct Rejectpacket),0,(struct sockaddr *)&serverStorage,addr_size);
			printf("....  OUT OF SEQUENCE ERROR .... \n\n");
		}
		else 
		{
			ackData = generateAckpacket(data);
			sendto(sockfd,&ackData,sizeof(struct Ackpacket),0,(struct sockaddr *)&serverStorage,addr_size);
			expectedPacket++;
			buffer[data.segment_number]++;
		}
		
		printf(".........................................................................................\n");
	}
	shutdown(sockfd, SHUT_RDWR);
}
