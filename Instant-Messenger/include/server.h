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
		int sockfd;


	public:
		Server();
		int prepareConnection(struct sockaddr_in serv_addr);
		int printPortNumber(struct sockaddr_in serv_addr);
		int ConnectToClient(pthread_t *tid);
		static void* clientCommunication(void *newsocket);	
};
	


		




