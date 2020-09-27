#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <vector>
#include <algorithm>

#define MAXBACKLOG SOMAXCONN


class Server
{
	private:
		int socket_fd;
		struct sockaddr_in serv_addr;
		//TODO maybe remove static and evaluate how to share vector between threads
	public:
		static std::vector <int> openSockets;
	public:
		Server();
		void setPort(int port);
		void prepareConnection();
		void printPortNumber();
		int ConnectToClient(pthread_t *tid);
		static void* clientCommunication(void *newsocket);
		static void closeClientConnection(int socket_fd);
		void closeConnections();
		void closeSocket();
		void closeServer();
};
