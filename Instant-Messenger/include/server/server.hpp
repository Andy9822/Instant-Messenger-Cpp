#ifndef SERVER_HPP
#define SERVER_HPP

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
#include "server_message_manager.hpp"
#include "../util/Socket.hpp"
#include "../util/Semaphore.hpp"
#include "ConnectionMonitor.hpp"

#define MAXBACKLOG SOMAXCONN

using namespace servergroupmanager;

namespace server {
    class Server : public Socket {
    private:
        ServerGroupManager *groupManager;
        ConnectionMonitor *connectionMonitor;

        struct sockaddr_in serv_addr;
        void closeClientConnection(pair<char *, int> clientConnectionId);
        std::map<string, int> connectionsCount;
        int limitOfConnectios;
        static void * monitorConnection(void *args);
        void closeFrontEndConnection(int socketId);
        pthread_t tid[MAXBACKLOG];


    public:
        Server();
        vector<int> socketFeList;
        static std::vector<int> openSockets;
        Semaphore* sockets_connections_semaphore;
        Semaphore* feConnectionInitializationSemaphore;
        Semaphore* feSocketsSemaphore;
        static void closeClientConnection(int socket_fd);
        static void *listenFrontEndCommunication(void *newsocket);
        void setPort(int port);
        void prepareConnection();
        void printPortNumber();
        int registerUserToServer(Packet *registrationPacket, int frontEndSocket);
        int registerUser(pair<char *, int> clientIdentifier, char *username, char *group);
        int connectToFE(string feAddress, int fePort);
        int handleFrontEndsConnections();
        void closeFrontEndConnections();
        void closeSocket(int socketId);
        void closeServer();
        void init_semaphore();
        void wait_semaphore();
        void post_semaphore();
        void configureFilesystemManager(int maxNumberOfMessagesInHistory);
        int getNumberOfConnectionsByUser(string user);
        int incrementNumberOfConnectionsFromUser(string user);

        void eraseSocketFromFeSocketList(int socketId);
    };
}
#endif