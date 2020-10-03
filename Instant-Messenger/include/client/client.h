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
#include "../util/Socket.hpp"

using clientcommunicationmanager::ClientCommunicationManager;
using clientmessagemanager::ClientMessageManager;
using userinterface::UserInteface;

class Client : public Socket
{
	private:
		string username;
		int sockfd;
		struct sockaddr_in serv_addr;
		ClientCommunicationManager clientCommunicationManager;
		ClientMessageManager clientMessageManager;
		UserInteface userInteface;
		
		string readInput();
		Packet buildPacket(string username, char* group, string input);
		void showMessage(Packet* receivedPacket);
	public:
		Client(char *ip_address, char *port);
        static void * receiveFromServer(void* args);
        static void * sendToServer(void* args);
		void setUsername(char* username);
		int ConnectToServer(char* username, char* group);
		int registerToServer(char* username, char* group);
		int clientCommunication(char* username, char* group);	
};