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
#include "client_communication_manager.h"
#include "client_message_manager.h"
#include "user_interface.h"

using clientcommunicationmanager::ClientCommunicationManager;
using clientmessagemanager::ClientMessageManager;
using userinterface::UserInteface;

class Client
{
	private:
		int sockfd;
		ClientCommunicationManager clientCommunicationManager;
		ClientMessageManager clientMessageManager;
		UserInteface userInteface;
	public:
		Client();
		int ConnectToServer(struct sockaddr_in serv_addr);
		int clientCommunication();	
};