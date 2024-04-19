#include<sys/socket.h>
//sockaddr_in used. IP networking			
#include<netinet/in.h>			
#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<string.h>
#include<time.h>
#include<signal.h>
#include "packetDefs.h"

int sockfd;
//Socket Client Shutdown when SIGINT(CTRL+C) is received
void intHandler(int dummy) {
	signal(SIGINT, SIG_IGN);
	printf("\nShutting Down Client\n");
	// closing the connection
	shutdown(sockfd, SHUT_RDWR);
	exit(0);
}
// Function to initialize the data packet
struct Datapacket initializeDataPacket() {
	struct Datapacket data;
	data.startPacketID = START_PACKET_ID;
	data.clientID = CLIENT_ID;
	data.type = DATA_PACKET_TYPE;
	data.endpacketID = END_PACKET_ID;
	return data;
}
// Function to initialize the ack packet
struct Ackpacket initializeAckPacket() {
	struct Ackpacket data;
	data.startPacketID = START_PACKET_ID;
	data.clientID = CLIENT_ID;
	data.type = ACK_PACKET_TYPE;
	data.endpacketID = END_PACKET_ID;
	return data;
}
// Function to initialize the reject packet
struct Rejectpacket initializerejectpacket() {
	struct Rejectpacket data;
	data.startPacketID = START_PACKET_ID;
	data.clientID = CLIENT_ID;
	data.type = ACK_PACKET_TYPE;
	data.endpacketID = END_PACKET_ID;
	return data;
}
// Function to print Client Data Packet information
void dataprint(struct Datapacket data) {
    printf("\n INFO: Sending datapacket:\n");
	printf("Packet ID: %x\n",data.startPacketID);
	printf("Client ID: %hhx\n",data.clientID);
	printf("Data: %x\n",data.type);
	printf("Segment Number: %d \n",data.segment_number);
	printf("Length: %d\n",data.length);
	printf("Payload: %s",data.payload);
	printf("End of Packet ID: %x\n",data.endpacketID);
	printf("\n");
}

// Function to print Server ACK Packet information
void ackprint(struct Ackpacket ackData) {
    printf("\n INFO: Received ackpacket:\n");
	printf("Packet ID: %x\n",ackData.startPacketID);
	printf("Client ID: %x\n",ackData.clientID);
	printf("ACK of Packet: %x\n",ackData.type);
	printf("Segment Number: %d \n",ackData.segment_number);
	printf("End of Packet ID: %x\n",ackData.endpacketID);
	printf("\n");
}

// Function to print Server REJECT Packet information
void rejectprint(struct Rejectpacket rejectData) {
    printf("\n INFO: Received rejectpacket:\n");
	printf("Packet ID: %x\n",rejectData.startPacketID);
	printf("Client ID: %hhx\n",rejectData.clientID);
	printf("Reject Type of Packet: %x\n",rejectData.type);
	printf("Segment Number: %d \n",rejectData.segment_number);
	printf("Reject : fff3\n");
	printf("Subcode : %x\n",rejectData.subcode);
	printf("End of Packet ID: %x\n",rejectData.endpacketID);
	printf("\n");
}


int main(){
	struct Datapacket data;
	struct Rejectpacket recievedpacket;
	// For IP networking, we use struct sockaddr_in
	struct sockaddr_in clientaddr;	
	struct Ackpacket ackdata;
	struct Rejectpacket rej;
	socklen_t addr_size;
	FILE *fp;


	char line[255];
	int n = 0;
	int counter = 0;
	int segmentNo = 1;
	int packetCount = 0;
	//Sig Interrupt
	signal(SIGINT, intHandler);

	//establish connection to socket AF_INET: SOCK_DGRAM (UDP Datagram Service) 
	sockfd = socket(AF_INET,SOCK_DGRAM,0);  
	if(sockfd < 0) {
		printf("....  Connection to Socket FAILED .... \n");
	}
	//Three key parts to Set and define connection
	bzero(&clientaddr,sizeof(clientaddr));
	//1. Address Family Ipv4. 
	clientaddr.sin_family = AF_INET;	
	//2. Address for the socket. Machine's IP Address.
	clientaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	//3. The Port number.
	clientaddr.sin_port=htons(PORT_NUMBER);	
	addr_size = sizeof clientaddr ;
	
	struct timeval tv;
	tv.tv_sec = TIMEOUT_SECONDS;
	tv.tv_usec = 0;
	//Set timeout on socket option
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval)); 
	
	
	fp = fopen("input.txt", "rt");
	if(fp == NULL)
	{
		printf("FILE OPEN EXCEPTION - Cannot Open input.txt file\n");
		//Closing the socket connection 
		shutdown(sockfd, SHUT_RDWR);
		exit(0);
	}
	
	while(1)
	{
		printf("\n....................................................................................\n");
		printf("Please select any one option\n");
		printf("1. Client: Send 5 Packets - Server Acknowledges for all 5 packets\n");
		printf("2. Client: Send 5 Packets - Server Acknowledges 4 with Error packets, 1 ACK Packet\n");
		printf("3. Client : Send single packet - Server send Reject Packet :Length Mismatch Error \n");
		printf("4. Client: Send single packet - Client: Send single packet -End of Packet Error \n");
		printf("5. Client: Send single packet - Client: Send single packet - Duplicate Packet Error \n");
		printf("\n.....................................................................................\n");
		
		int i;
		scanf("%d",&i);
		
		switch (i)
		{
		case 1:
			for(packetCount=0;packetCount<5;packetCount++){
				data = initializeDataPacket();
				ackdata = initializeAckPacket();
				rej=initializerejectpacket();
				if(fgets(line, sizeof(line), fp) != NULL) 
				{
					n = 0;
					counter = 0;
					printf("%s",line);
					data.segment_number = segmentNo;
					strcpy(data.payload,line);     
					data.length = strlen(data.payload);
					data.endpacketID = END_PACKET_ID;
				}
				while(n<=0 && counter<3)
				{
					//SockADDR: Container that helps OS in identifying the Address family from the first 2 bytes. (Ipv4 here)
					sendto(sockfd,&data,sizeof(struct Datapacket),0,(struct sockaddr *)&clientaddr,addr_size);
					n = recvfrom(sockfd,&recievedpacket, sizeof(struct Rejectpacket),0,NULL,NULL);
					if(n <= 0 )
					{
						printf("No response from server for %d seconds. Sending the packet again.\n",TIMEOUT_SECONDS);
						counter ++;
					}
					else if(recievedpacket.type == ACK_PACKET_TYPE  ) 
					{
						dataprint(data);
						printf("ACKNOWLEDGEMENT packet recieved \n");
						ackdata.segment_number = data.segment_number;
						ackprint(ackdata);
						printf("\n");
						segmentNo++;
					}
					else if(recievedpacket.type == REJECT_PACKET_TYPE ) 
					{
						printf("REJECT Packet recieved \n");
						rej.subcode=recievedpacket.subcode;
						rej.segment_number = data.segment_number;
						if(recievedpacket.subcode == LENGTH_MISMATCH_CODE ) 
						{
							printf(".... LENGTH MISMATCH ERROR ....\n");
						}
						if(recievedpacket.subcode == END_MISSING_CODE ) 
						{
							printf(".... END OF PACKET IDENTIFIER MISSING ERROR ....\n");
						}
						if(recievedpacket.subcode == OUT_OF_SEQUENCE_CODE ) 
						{
							printf(".... OUT OF SEQUENCE ERROR ....\n");
						}
						if(recievedpacket.subcode == DUPLICATE_PACKET_CODE) 
						{
							printf(".... DUPLICATE PACKET ERROR ....\n");
						}
						rejectprint(rej);
					}
				}
				if(counter >= 3 ) 
				{
					printf("Server does not respond. Server is down. Please try again later.\n");
					//Closing the socket connection 
					shutdown(sockfd, SHUT_RDWR);
					exit(0);
				}
				printf("----------------------------------------------------------------------------------------\n");
			}
			break;
					
		case 3:
			data = initializeDataPacket();
			ackdata = initializeAckPacket();
			rej=initializerejectpacket();
			if(fgets(line, sizeof(line), fp) != NULL) 
			{
				n = 0;
				counter = 0;
				printf("%s",line);
				data.segment_number = segmentNo;
				strcpy(data.payload,line);
				data.length = strlen(data.payload);
				data.endpacketID = END_PACKET_ID;
			}
			// Changing length of data
			data.length++;   
			while(n<=0 && counter<3)
			{
				sendto(sockfd,&data,sizeof(struct Datapacket),0,(struct sockaddr *)&clientaddr,addr_size);
				n = recvfrom(sockfd,&recievedpacket,sizeof(struct Rejectpacket),0,NULL,NULL);
				if(n <= 0 )
				{
					printf("No response from server for %d seconds, sending the packet again\n",TIMEOUT_SECONDS);
					counter ++;
				}
				else if(recievedpacket.type == ACK_PACKET_TYPE ) 
				{
					dataprint(data);
					printf("ACKNOWLEDGEMENT packet recieved \n ");
					ackdata.segment_number = data.segment_number;
					ackprint(ackdata);
					segmentNo++;
				}
				else if(recievedpacket.type == REJECT_PACKET_TYPE ) 
				{
					printf("REJECT Packet recieved \n");
					rej.subcode=recievedpacket.subcode;
					rej.segment_number = data.segment_number;
					if(recievedpacket.subcode == LENGTH_MISMATCH_CODE ) 
					{
						printf(".... LENGTH MISMATCH ERROR ....\n");
					}
					if(recievedpacket.subcode == END_MISSING_CODE ) 
					{
						printf(".... END OF PACKET IDENTIFIER MISSING ERROR ....\n");
					}
					if(recievedpacket.subcode == OUT_OF_SEQUENCE_CODE ) 
					{
						printf(".... OUT OF SEQUENCE ERROR ....\n");
					}
					if(recievedpacket.subcode == DUPLICATE_PACKET_CODE) 
					{
						printf(".... DUPLICATE PACKET ERROR ....\n");
					}
					rejectprint(rej);
				}
			}
			if(counter >= 3 ) 
			{
				printf("Server does not respond. Server is down for a while. Please try again later.\n");
				//Closing the socket connection 
				shutdown(sockfd, SHUT_RDWR);
				exit(0);
			}
			
			printf("----------------------------------------------------------------------------------------\n");
			break;

		case 4:
			data = initializeDataPacket();
			ackdata = initializeAckPacket();
			rej=initializerejectpacket();
			if(fgets(line, sizeof(line), fp) != NULL) 
			{
				n = 0;
				counter = 0;
				printf("%s",line);
				data.segment_number = segmentNo;
				strcpy(data.payload,line);
				data.length = strlen(data.payload);
				data.endpacketID = END_PACKET_ID;
			}
			data.endpacketID= 0;  
			while(n<=0 && counter<3)
			{
				sendto(sockfd,&data,sizeof(struct Datapacket),0,(struct sockaddr *)&clientaddr,addr_size);
				n = recvfrom(sockfd,&recievedpacket,sizeof(struct Rejectpacket),0,NULL,NULL);
				if(n <= 0 )
				{
					printf("No response from server for %d seconds sending the packet again\n",TIMEOUT_SECONDS);
					counter ++;
				}
				else if(recievedpacket.type == ACK_PACKET_TYPE  ) 
				{
					dataprint(data);
					printf("ACKNOWLEDGEMENT packet recieved \n ");
					ackdata.segment_number = data.segment_number;
					ackprint(ackdata);
					segmentNo++;
				}
				else if(recievedpacket.type == REJECT_PACKET_TYPE ) 
				{
					printf("REJECT Packet recieved \n");
					rej.subcode=recievedpacket.subcode;
					rej.segment_number = data.segment_number;
					if(recievedpacket.subcode == LENGTH_MISMATCH_CODE ) 
					{
						printf(".... LENGTH MISMATCH ERROR ....\n");
					}
					if(recievedpacket.subcode == END_MISSING_CODE ) 
					{
						printf(".... END OF PACKET IDENTIFIER MISSING ERROR ....\n");
					}
					if(recievedpacket.subcode == OUT_OF_SEQUENCE_CODE ) 
					{
						printf(".... OUT OF SEQUENCE ERROR ....\n");
					}
					if(recievedpacket.subcode == DUPLICATE_PACKET_CODE) 
					{
						printf(".... DUPLICATE PACKET ERROR ....\n");
					}
					rejectprint(rej);
				}
			}
			if(counter >= 3 ) 
			{
				printf("Server does not respond. Server is down for a while. Please try again later.\n");
				//Closing the socket connection 
				shutdown(sockfd, SHUT_RDWR);
				exit(0);
			}
			
			printf("----------------------------------------------------------------------------------------\n");
			break;

		case 5:
			data = initializeDataPacket();
			ackdata = initializeAckPacket();
			rej=initializerejectpacket();
			if(fgets(line, sizeof(line), fp) != NULL) 
			{
				n = 0;
				counter = 0;
				printf("%s",line);
				data.segment_number = segmentNo;
				strcpy(data.payload,line);
				data.length = strlen(data.payload);
				data.endpacketID = END_PACKET_ID;
			}
			data.segment_number = 3;
			while(n<=0 && counter<3)
			{
				sendto(sockfd,&data,sizeof(struct Datapacket),0,(struct sockaddr *)&clientaddr,addr_size);
				n = recvfrom(sockfd,&recievedpacket,sizeof(struct Rejectpacket),0,NULL,NULL);
				if(n <= 0)
				{
					printf("ERROR! No response from server for %d seconds sending the packet again\n",TIMEOUT_SECONDS);
					counter ++;
				}
				else if(recievedpacket.type == ACK_PACKET_TYPE  ) 
				{
					dataprint(data);
					printf("ACKNOWLEDGEMENT packet recieved \n ");
					ackdata.segment_number = data.segment_number - 1;
					ackprint(ackdata);
					segmentNo++;
				}
				else if(recievedpacket.type == REJECT_PACKET_TYPE ) 
				{
					printf("REJECT Packet recieved \n");
					rej.subcode=recievedpacket.subcode;
					rej.segment_number = data.segment_number;
					if(recievedpacket.subcode == LENGTH_MISMATCH_CODE ) 
					{
						printf(".... LENGTH MISMATCH ERROR ....\n");
					}
					if(recievedpacket.subcode == END_MISSING_CODE ) 
					{
						printf(".... END OF PACKET IDENTIFIER MISSING ERROR ....\n");
					}
					if(recievedpacket.subcode == OUT_OF_SEQUENCE_CODE ) 
					{
						printf(".... OUT OF SEQUENCE ERROR ....\n");
					}
					if(recievedpacket.subcode == DUPLICATE_PACKET_CODE) 
					{
						printf(".... DUPLICATE PACKET ERROR ....\n");
					}
					rejectprint(rej);
				}
			}
			if(counter >= 3 ) 
			{
				printf("Server does not respond. Server is down for a while. Please try again later.\n");
				//Closing the socket connection 
				shutdown(sockfd, SHUT_RDWR);
				exit(0);
			}
			
			printf("----------------------------------------------------------------------------------------\n");
			break;
		
		case 2:
			for(packetCount=0;packetCount<5;packetCount++){
				data = initializeDataPacket();
				ackdata = initializeAckPacket();
				rej=initializerejectpacket();
				if(fgets(line, sizeof(line), fp) != NULL) 
				{
					n = 0;
					counter = 0;
					printf("%s",line);
					data.segment_number = segmentNo;
					strcpy(data.payload,line);
					data.length = strlen(data.payload);
					data.endpacketID = END_PACKET_ID;
				}
				switch(packetCount+1){
					case 2:
						data.length++;
						break;
					case 1:
						data.endpacketID= 0;
						break;
					case 3:
						data.segment_number = 20;
						break;
					case 5:
						data.segment_number = 3;
						break;
				}
				
				while(n<=0 && counter<3)
				{
					sendto(sockfd,&data,sizeof(struct Datapacket),0,(struct sockaddr *)&clientaddr,addr_size);
					n = recvfrom(sockfd,&recievedpacket,sizeof(struct Rejectpacket),0,NULL,NULL);
					if(n <= 0 )
					{
						printf("No response from server for %d seconds sending the packet again\n",TIMEOUT_SECONDS);
						counter ++;
					}
					else if(recievedpacket.type == ACK_PACKET_TYPE) 
					{
						dataprint(data);
						printf("ACKNOWLEDGEMENT packet recieved \n ");
						ackdata.segment_number = data.segment_number;
						ackprint(ackdata);
						segmentNo++;
					}
					else if(recievedpacket.type == REJECT_PACKET_TYPE ) 
					{
						printf("REJECT Packet recieved \n");
						rej.subcode=recievedpacket.subcode;
						rej.segment_number = data.segment_number;
						if(recievedpacket.subcode == LENGTH_MISMATCH_CODE ) 
						{
							printf("xxx LENGTH MISMATCH ERROR xxx\n");
						}
						if(recievedpacket.subcode == END_MISSING_CODE ) 
						{
							printf("xxx END OF PACKET IDENTIFIER MISSING ERROR xxx \n");
						}
						if(recievedpacket.subcode == OUT_OF_SEQUENCE_CODE ) 
						{
							printf("xxx OUT OF SEQUENCE ERROR xxx\n");
						}
						if(recievedpacket.subcode == DUPLICATE_PACKET_CODE) 
						{
							printf("xxx DUPLICATE PACKET ERROR xxx\n");
						}
						rejectprint(rej);
					}
				}
				if(counter >= 3 ) 
				{
					printf("xxxx ERROR! Server is down. Please try again later. xxxx\n");
					//Closing the socket connection 
					shutdown(sockfd, SHUT_RDWR);
					exit(0);
				}
				
				printf("----------------------------------------------------------------------------------------\n");
			}
			break;

		default:
			printf("\n \n Invalid Option \n \n!");
		}
	}
}
