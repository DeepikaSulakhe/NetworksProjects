
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <strings.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "packterDefs.h"

int sockfd;

void intHandler(int dummy) {
	signal(SIGINT, SIG_IGN);
	printf("\nShutting Down Server\n");
	shutdown(sockfd, SHUT_RDWR);
	exit(0);
}

// Subscriber  Information
struct SubscriberDatabase {
	unsigned long subscriberNumber;
	uint8_t technology;
	int status;

};


// Function For Printing Packet Contents
void displayPacket(struct RequestPacket *requestPacket) {
	printf("\nStart of Packet ID: %x\n",requestPacket->startPacketID);
	printf("Client ID: %hhx\n",requestPacket->clientID);
	printf("Access Permission: %x\n",requestPacket->acc_Per);
	printf("Segment Number: %d \n",requestPacket->segment_Num);
	printf("Length: %d\n",requestPacket->length);
	printf("Technology: %d \n", requestPacket->technology);
	printf("Source Subscriber Number: %u \n",requestPacket->sourceSubscriberNum);
	printf("End of Packet ID : %x \n",requestPacket->endPacketID);
}


// Generate  Response  Packet
struct ResponsePacket generateResponsePacket(struct RequestPacket requestPacket) {
	struct ResponsePacket responsePacket;
	responsePacket.startPacketID = requestPacket.startPacketID;
	responsePacket.clientID = requestPacket.clientID;
	responsePacket.segment_Num = requestPacket.segment_Num;
	responsePacket.length = requestPacket.length;
	responsePacket.technology = requestPacket.technology;
	responsePacket.sourceSubscriberNum = requestPacket.sourceSubscriberNum;
	responsePacket.endPacketID = requestPacket.endPacketID;
	return responsePacket;
}

// Read File for Mapping
void readFile(struct SubscriberDatabase subscriberDB[]) {

	//Read file and store locally
	char line[30];
	int i = 0;
	FILE *filePointer;

	filePointer = fopen("Verification_Database.txt", "rt");

	if(filePointer == NULL)
	{
		printf("\n ERROR! File not found \n");
		return;
	}
	while(fgets(line, sizeof(line), filePointer) != NULL)
	{
		char * words=NULL;
		words = strtok(line," ");
		subscriberDB[i].subscriberNumber =(unsigned) atol(words);   
		words = strtok(NULL," ");
		subscriberDB[i].technology = atoi(words);						
		words = strtok(NULL," ");
		subscriberDB[i].status = atoi(words);
		i++;
	}
	fclose(filePointer);
}


// Check if Subscriber exists
int check(struct SubscriberDatabase subscriberDB[],unsigned int subscriberNumber,uint8_t technology) {
	int value = -1;
	for(int j = 0; j < LENGTH;j++) {
		if(subscriberDB[j].subscriberNumber == subscriberNumber && subscriberDB[j].technology == technology) {
			return subscriberDB[j].status;
		}
                else if (subscriberDB[j].subscriberNumber == subscriberNumber && subscriberDB[j].technology != technology)
                        return 2;
	}
	return value;
}


int main(){
	
    struct RequestPacket requestPkt;
	struct ResponsePacket responsePkt;
	
    struct SubscriberDatabase subscriberDB[LENGTH];
	readFile(subscriberDB);
	
    int n;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;
	signal(SIGINT, intHandler);
	sockfd=socket(AF_INET,SOCK_DGRAM,0);
	
	
    bzero(&serverAddr,sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	serverAddr.sin_port=htons(PORT);
	
    bind(sockfd,(struct sockaddr *)&serverAddr,sizeof(serverAddr));
	addr_size = sizeof serverAddr;
	printf("Server Running\n");
	// start receiving from client 
    while(1) {
                // Get the packet
		n = recvfrom(sockfd,&requestPkt,sizeof(struct RequestPacket),0,(struct sockaddr *)&serverStorage, &addr_size);
		displayPacket(&requestPkt);
		if(requestPkt.segment_Num == 11) {
			shutdown(sockfd, SHUT_RDWR);
			exit(0);
		}

		if(n > 0 && requestPkt.acc_Per == ACCESS_PERM) {
			// Response Packet
			responsePkt = generateResponsePacket(requestPkt);

			int value = check(subscriberDB,requestPkt.sourceSubscriberNum,requestPkt.technology);
			if(value == 0) {
				
				responsePkt.type = NOTPAID;
				printf("SUBSCRIBER HAS NOT PAID\n");
			}
			else if(value == 1) {
				
				printf("SUBSCRIBER HAS PAID\n");
				responsePkt.type = PAID;
			}

			else if(value == -1) {
               
				printf("SUBSCRIBER DOES NOT EXIST ON THE DATABASE\n");
				responsePkt.type = NOTEXIST;
			}
			else{
				printf("SUBCRIBER TECHNOLOGY MISMATCH\n");
                responsePkt.type = NOTEXIST;
			}                        
			// sending the response packet
			sendto(sockfd,&responsePkt,sizeof(struct ResponsePacket),0,(struct sockaddr *)&serverStorage,addr_size);
		}
		n = 0;
		printf("\n-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ \n");
	}
}
