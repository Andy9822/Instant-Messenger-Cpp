#include "../../include/client/client.h"
#include "../../include/util/Packet.hpp"



Client::Client(char *ip_address, char *port)
{
	sockfd = 0;

	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(atoi(port));    
	
	if(inet_pton(AF_INET, ip_address, &serv_addr.sin_addr)<=0) 
	{ 
		std::cout << "\nInvalid address/ Address not supported \n" << std::endl; 
	} 

	bzero(&(serv_addr.sin_zero), 8);  
}



int Client::ConnectToServer()
{
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "\n Socket creation error \n" << endl;
		return -1;
	}

	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
	{
        cout << "ERROR connecting\n" << endl;
        return -1;
	}

	return 0;
}



Packet buildPacket(char* group, string input)
{
	char messageBuffer[256] = {0};
	char groupBuffer[256] = {0};

	strncpy(groupBuffer, group, 19);
	strncpy(messageBuffer, input.c_str(), 255); //Send message with maximum of 255 characters
	return Packet(groupBuffer, messageBuffer);
}



int Client::clientCommunication(char* group)
{
	int n;
	char input[255];
	Packet *sendingPacket = new Packet();
	int packetSize = sizeof(Packet);
	bool connectedToServer = true;

	while(connectedToServer)
    {
		// Read input
    	cout << "Enter the message: ";
    	bzero(input, 255);

    	if(fgets(input, 255, stdin) == NULL) // ctrl+d
    	{
    		break;
    	}

		// Prepare Packet struct to be sent
		*sendingPacket = buildPacket(group, input);

		//Send Packet struct via TCP socket
		sendPacket(sockfd, sendingPacket);


		// Listen from TCP connection in case a Packet is received
		Packet* receivedPacket = readPacket(sockfd, &connectedToServer);

		if (!connectedToServer)
		{
			// Free allocated memory for reading Packet
			free(sendingPacket);
			break;
		}

		cout << "[Server Message]: " << receivedPacket->message  << endl;
	}

	cout << endl;
	close(sockfd);

	return 0;
}
