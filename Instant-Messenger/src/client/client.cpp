#include "../../include/client/client.h"
#include "../../include/util/Packet.hpp"



Client::Client()
{
	sockfd = 0;
}



int Client::ConnectToServer(struct sockaddr_in serv_addr)
{
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		std::cout << "\n Socket creation error \n" << std::endl;
		return -1;
	}

	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
	{
        std::cout << "ERROR connecting\n" << std::endl;;
        return -1;
	}

	return 0;
}



int Client::clientCommunication()
{
	char buffer[256];
	int n;

	while(1)
    {
    	std::cout << "Enter the message: ";

    	bzero(buffer, 256);

    	if(fgets(buffer, 256, stdin) == NULL) // ctrl+d
    	{
    		break;
    	}

		Packet* mypacket = new Packet("Grupo dos guri", buffer);
		n = write(sockfd, (void *) mypacket, sizeof(Packet));

	    if (n < 0)
	    {
			std::cout << "ERROR writing to socket\n" << std::endl;
			return -1;
	    }



			// Read from server
			int readBytes = 0;
			int packetSize = sizeof(Packet);
			void *packet = (void*) malloc(packetSize);

	    bzero(buffer,256);

			do {
				n = read(sockfd, (packet + readBytes), packetSize-readBytes);


				readBytes += n;
			} while ( n > 0);

			Packet* receivedPacket = (Packet*) packet;
			cout << "[Server Message]: " << receivedPacket->message  << endl;

	    if (n < 0)
	    {
				std::cout << "ERROR reading from socket\n" << std::endl;
				return -1;
	    }
	}

	close(sockfd);

	return 0;
}
