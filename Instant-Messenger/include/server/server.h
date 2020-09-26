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

#define MAXBACKLOG SOMAXCONN


class Server
{
	private:
		int socket_fd;
		struct sockaddr_in serv_addr;

	public:
		Server(int port);
		void prepareConnection();
		void printPortNumber();
		int ConnectToClient(pthread_t *tid);
		static void* clientCommunication(void *newsocket);
};
