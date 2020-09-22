#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>


class Client
{
	private:
		int sockfd;


	public:
		Client();
		int ConnectToServer(struct sockaddr_in serv_addr);
		int clientCommunication();	
};






