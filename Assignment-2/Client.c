#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include "packterDefs.h"


// Function For Printing Packet Contents
void DisplayPacket(struct RequestPacket *requestPacket) {
	printf(" Start of Packet ID: %x\n",requestPacket->startPacketID);
	printf("Client ID: %hhx\n",requestPacket->clientID);
	printf("Access Permission: %x\n",requestPacket->acc_Per);
	printf("Segment Number: %d \n",requestPacket->segment_Num);
	printf("Length: %d\n",requestPacket->length);
	printf("Technology: %d \n", requestPacket->technology);
	printf("Subscriber Number: %u \n",requestPacket->sourceSubscriberNum);
	printf("End of Packet ID: %x \n",requestPacket->endPacketID);
}


// Intializing Data To Request Packet
struct RequestPacket Initialize () {
	struct RequestPacket requestPkt;
	requestPkt.startPacketID = 0XFFFF;
	requestPkt.clientID = 0XFF;
	requestPkt.acc_Per = 0XFFF8;
	requestPkt.endPacketID = 0XFFFF;
	return requestPkt;

}

int main(){
	struct RequestPacket requestPkt;
	struct ResponsePacket responsePkt;
	char line[30];
	int i = 1;
	FILE *filePointer;
	int sockfd,n = 0;
	struct sockaddr_in clientAddr;
	socklen_t addr_size;
	sockfd = socket(AF_INET,SOCK_DGRAM,0);
	struct timeval timeValue;
	timeValue.tv_sec = 3;  // Timeout
	timeValue.tv_usec = 0;

	// Checking for connection
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeValue,sizeof(struct timeval));
	int counter = 0;
	if(sockfd < 0) {
		printf("Socket Connection Failed\n");
	}
	bzero(&clientAddr,sizeof(clientAddr));
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	clientAddr.sin_port=htons(PORT);
	addr_size = sizeof clientAddr ;


	// Loading of data into Packet
	requestPkt = Initialize();

	//Read input file

	filePointer = fopen("input.txt", "rt");

	if(filePointer == NULL)
	{
		printf("Cannot Open The Input File!\n");
		shutdown(sockfd, SHUT_RDWR);
		exit(0);
	}


	while(fgets(line, sizeof(line), filePointer) != NULL) {
		counter = 0;
		n = 0;
		printf(" \n***  ->  NEW REQUEST  <-  ***\n\n");
		char * words;
		//Split the line
		words = strtok(line," ");
		requestPkt.length = strlen(words);
		requestPkt.sourceSubscriberNum = (unsigned) atoi(words);
		words = strtok(NULL," ");
		requestPkt.length += strlen(words);
		requestPkt.technology = atoi(words);
		words = strtok(NULL," ");
		requestPkt.segment_Num = i;
		// Printing Contents of the Packet
		DisplayPacket(&requestPkt);
		while(n <= 0 && counter < 3) { 
			// Check if packet sent, if not tries again.
			sendto(sockfd,&requestPkt,sizeof(struct RequestPacket),0,(struct sockaddr *)&clientAddr,addr_size);
			//  Get response from Server
			n = recvfrom(sockfd,&responsePkt,sizeof(struct ResponsePacket),0,NULL,NULL);
			if(n <= 0 ) {
				// No response has received
				printf("Server TIMEOUT\n");
				counter ++;
			}
			else if(n > 0) {
				// Response recieved
				printf("\n..............................................\n");
				printf("Status = ");
				if(responsePkt.type == NOTPAID) {
					printf("SUBSCRIBER HAS NOT PAID\n");
					printf("..............................................\n");
				}
				else if(responsePkt.type == NOTEXIST ) {
					printf("SUBSCRIBER DOES NOT EXIST ON DATABASE\n");
					printf("..............................................\n");
				}
				else if(responsePkt.type == PAID) {
					printf("SUBSCRIBER PERMITTED TO ACCESS THE NETWORK\n");
					printf("..............................................\n");

				}
			}
		}
		// After 3 attempts
		if(counter >= 3 ) {
			printf("SERVER NOT RESPONDING");
			exit(0);
		}
		i++;
		printf("\n------------------------------------------------------------------------------------ \n");
	}
	fclose(filePointer);
	shutdown(sockfd, SHUT_RDWR);
}
