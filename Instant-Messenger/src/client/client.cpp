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



int Client::clientCommunication()
{
	int n;
	char buffer[256] = {0};
	Packet *mypacket = new Packet((char*)"", buffer);
	Packet *receivedPacket = new Packet();
	int packetSize = sizeof(Packet);

	while(1)
    {
    	cout << "Enter the message: ";

    	bzero(buffer, 256);

    	if(fgets(buffer, 256, stdin) == NULL) // ctrl+d
    	{
    		break;
    	}

		*mypacket = Packet((char*)"Grupo dos guri", buffer);

		n = write(sockfd, mypacket, packetSize);

	    if (n < 0)
	    {
			cout << "ERROR writing to socket\n" << endl;
			return -1;
	    }

		n = read(sockfd, receivedPacket, packetSize);

		cout << "[Server Message]: " << receivedPacket->message  << endl;

	    if (n < 0)
	    {
			cout << "ERROR reading from socket\n" << endl;
			return -1;
	    }

	    *receivedPacket = Packet();
	}

	delete mypacket;
	delete receivedPacket;
	close(sockfd);

	return 0;
}
