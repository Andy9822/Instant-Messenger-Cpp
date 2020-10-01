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
#include <semaphore.h>
#include "server_group_manager.hpp"
#include "../util/Socket.hpp"

#define MAXBACKLOG SOMAXCONN


class Server: public Socket
{
	private:
		ServerGroupManager *groupManager;
		int socket_fd;
		struct sockaddr_in serv_addr;
		//TODO maybe remove static and evaluate how to share vector between threads
	public:
		static std::vector <int> openSockets;
		static sem_t semaphore;
		
	public:
		Server();
		void setPort(int port);
		void prepareConnection();
		void printPortNumber();
		static void* registerUserToServer(void* socket);
		int registerUser(int socket, char* username, char* group);
		int handleClientConnection(pthread_t *tid);
		static void* clientCommunication(void *newsocket);
		static void closeClientConnection(int socket_fd);
		void closeConnections();
		void closeSocket();
		void closeServer();
		void init_semaphore();
		void wait_semaphore();
		void post_semaphore();
	private:
		void closeClientCommunication(int client_socket);
};
