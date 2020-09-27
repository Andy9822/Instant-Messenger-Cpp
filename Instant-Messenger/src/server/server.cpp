#include "../../include/server/server.h"
#include "../../include/util/Packet.hpp"

Server::Server()
{
	// Configure server address properties
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);

	// Initialize socket file descriptor
	socket_fd = 0;
}

void Server::closeConnections()
{
	cout << "Closing client connections..." << endl;
}

void Server::closeSocket()
{
	close(socket_fd);
	cout << "Closing server socket..." << endl;
}

void Server::closeServer() 
{
	closeConnections();
	closeSocket();
}

void Server::setPort(int port) 
{
	serv_addr.sin_port = htons(port);
}

void Server::prepareConnection()
{
	// Create socket file descriptor
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
  		cout << "ERROR opening socket\n" << endl;
  		exit(1);
	}

	// Attach socket to server's port
  if (bind(socket_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
  {
		cout << "ERROR on binding\n" << endl;
		exit(1);
  }

	// Configure socket to listen for tcp connections
	if (listen(socket_fd, MAXBACKLOG) < 0) // SOMAXCONN is the maximum value of backlog
	{
		cout << "ERROR on listening\n" << endl;
		exit(1);
	}
}



void Server::printPortNumber()
{
	socklen_t len = sizeof(serv_addr);

	if(getsockname(socket_fd, (struct sockaddr *)&serv_addr, &len) < 0)
	{
		cout << "Unable to print port Number!" << endl;
		exit(1);
	}

	cout << "Server running on PORT: " << ntohs(serv_addr.sin_port) << endl;
}



int Server::ConnectToClient(pthread_t *tid)
{
	int newsockfd;
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(struct sockaddr_in);
	Packet *packet = new Packet();

	if ((newsockfd = accept(socket_fd, (struct sockaddr *) &cli_addr, &clilen)) == -1)
	{
		cout << "ERROR on accept\n" << endl;
		return -1;
	}

	packet->clientSocket = newsockfd;

	pthread_create(tid, NULL, clientCommunication , packet);

	return 0;
}

void* Server::clientCommunication(void *packet)
{
	Packet *receivedPacket = (Packet*)packet;
	Packet *sendingPacket = new Packet((char*)"Grupo dos guri", (char*)"Recebi sua mensagem!");
	char buffer[256] = {0};
	int newsockfd = receivedPacket->clientSocket;
	int packetSize = sizeof(Packet);
	int n;

	while(1)
	{
		n = read(newsockfd, receivedPacket, packetSize);

		if (n < 0) 
		{
			cout << "ERROR reading from socket\n" << endl;
			break;
		}
		else if(n == 0) // Client exited with ctrl+d
		{
			cout << "End of connection with socket " << newsockfd << endl;
			break;
		}

		cout << "Room: " << receivedPacket->group  << endl;
		cout << "[Message]: " << receivedPacket->message  << endl;

		n = write(newsockfd, (void *) sendingPacket, packetSize);

		if (n < 0) 
		{
			cout << "ERROR writing to socket\n" << endl;
		}

		*receivedPacket = Packet();
	}

	delete sendingPacket;
	delete (Packet*)packet;
	close(newsockfd);

	return 0;
}
