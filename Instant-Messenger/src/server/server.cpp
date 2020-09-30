#include "../../include/server/server.h"
#include "../../include/util/Packet.hpp"

vector<int> Server::openSockets;
sem_t Server::semaphore;

Server::Server()
{
	// Configure server address properties
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);

	// Initialize socket file descriptor
	socket_fd = 0;
}

void Server::closeClientConnection(int socket_fd)
{
	std::cout << "Closing socket: " << socket_fd << std::endl;
	openSockets.erase(std::remove(openSockets.begin(), openSockets.end(), socket_fd), openSockets.end());
	close(socket_fd);
}

void Server::closeConnections()
{
	cout << "Number of client connections: " << openSockets.size() << endl;
	// std::for_each(openSockets.begin(), openSockets.end(), close);
	std::for_each(openSockets.begin(), openSockets.end(), closeClientConnection);
	cout << "Number of client connections: " << openSockets.size() << endl;
}

void Server::closeSocket()
{
	std::cout << "server_socket_fd: " << socket_fd << std::endl;
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

	// Init binary semaphore for openSockets
	init_semaphore();
	
	std::cout << "server_socket_fd preparedConnection: " << socket_fd << std::endl;
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



int Server::handleClientConnection(pthread_t *tid)
{
	// Allocate memory space to store value in heap and be able to use it after this function ends
	int* newsockfd = (int *) calloc(1,sizeof(int));
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(struct sockaddr_in);

	if ((*newsockfd = accept(socket_fd, (struct sockaddr *) &cli_addr, &clilen)) == -1)
	{
		cout << "ERROR on accept\n" << endl;
		return -1;
	}

	std::cout << "client connected  with socket: " << *newsockfd << std::endl;

	wait_semaphore();
	// Store new socket crated in the vector of existing connections sockets
	openSockets.push_back(*newsockfd);
	post_semaphore();
	
	// Send pointer to the previously allocated address and be able to access it's value in new thread's execution
	pthread_create(tid, NULL, clientCommunication , newsockfd);

	return 0;
}

void* Server::clientCommunication(void *socket_pointer)
{
	// Read value of received socket pointer and free the allocated memory previously in the main thread
	int socket_fd = *(int *) socket_pointer;
	free(socket_pointer);

	int packetSize = sizeof(Packet);
	void *incomingData = (void*) malloc(packetSize);
	
	bool connectedClient = true;
	int n;

	while(connectedClient)
	{
		int receivedBytes = 0;
		do {
			n = read(socket_fd, (incomingData + receivedBytes), packetSize-receivedBytes);
			if (n < 0) 
			{
				cout << "ERROR reading from socket\n" << endl;
				break;
			}
			else if(n == 0) // Client closed connection
			{
				connectedClient = false;
				cout << "End of connection with socket " << socket_fd << endl;
				break;
			}
			receivedBytes += n;
		} while ( receivedBytes != packetSize);
		if (!connectedClient)
		{
			break;
		}
		
		Packet* receivedPacket = (Packet*) incomingData;

		cout << "Room: " << receivedPacket->group  << endl;
		cout << "[Message]: " << receivedPacket->message  << endl;

		Packet* sendingPacket = new Packet(receivedPacket->group, "Recebi sua mensagem!");
		n = write(socket_fd, (void *) sendingPacket, packetSize);

		if (n < 0) 
		{
			cout << "ERROR writing to socket\n" << endl;
		}
	}

	std::cout << "Freeing allocated memory and closing client connection thread" << std::endl;

	Server::wait_semaphore();
	openSockets.erase(std::remove(openSockets.begin(), openSockets.end(), socket_fd), openSockets.end());
	Server::post_semaphore();
	free(incomingData);
	
	 if ( (close(socket_fd)) == 0 )
	 {
		std::cout << "Closed socket: " << socket_fd << std::endl;
	 }
	 else 
	 {	
		std::cout << "!!! Fatal error closing socket!!!!" << std::endl;
	 }
	 

	return 0;
}

void Server::init_semaphore() {
    sem_init(&semaphore, 0, 1);
}
void Server::wait_semaphore()
{
	sem_wait(&semaphore);
}
void Server::post_semaphore()
{
	sem_post(&semaphore);
}
