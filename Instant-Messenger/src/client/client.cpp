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



int *Client::ConnectToServer(Message userInfo)
{
	int success;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "\n Socket creation error \n" << endl;
		sockfd = -1;
	}

	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
	{
        cout << "ERROR connecting\n" << endl;
        sockfd = -1;
	}

	// Asking server if username already exists
	write(sockfd, &userInfo, sizeof(Message));
	read(sockfd, &success, sizeof(int));

	if(success == -1)
	{
		cout << "You are already logged in 2 sessions" << endl;
		sockfd = -1;
	}

	return &sockfd;
}



void* Client::writeToServer(void* socket)
{
	int sock_fd = *(int*)socket;
	char buffer[256] = {0};
	Packet *mypacket = new Packet((char*)"", buffer);
	int packetSize = sizeof(Packet);
	int n;

	
	while(1)
	{
		cout << "Enter the message: ";

	    bzero(buffer, 256);

	    if(fgets(buffer, 256, stdin) == NULL) // ctrl+d
	    {
	    	*mypacket = Packet((char*)"Grupo dos guri", buffer);
	    	write(sock_fd, mypacket, packetSize);
	    	break;
	    }

		*mypacket = Packet((char*)"Grupo dos guri", buffer);

		n = write(sock_fd, mypacket, packetSize);

		if (n < 0)
		{
			cout << "ERROR writing to socket\n" << endl;
			break;
		}
	}

	delete mypacket;
	close(sock_fd);

	return 0;
}



void* Client::ReadFromServer(void* socket)
{
	int sock_fd = *(int*)socket;
	Packet *receivedPacket = new Packet();
	int packetSize = sizeof(Packet);
	int n;

	while(1)
	{
		n = read(sock_fd, receivedPacket, packetSize);

		cout << "\n[Server Message]: " << receivedPacket->message  << endl;

		if (n < 0)
		{
			//cout << "\nERROR reading from socket\n" << endl;
			break;
		}

		*receivedPacket = Packet();
	}	

	delete receivedPacket;
	close(sock_fd);

}



/*int Client::clientCommunication()
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
*/