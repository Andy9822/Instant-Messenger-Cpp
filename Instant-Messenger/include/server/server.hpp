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
        bool isPrimaryServer;
        int rmNumber;
        int rm_listening_socket_fd;
        struct sockaddr_in rm_listening_serv_addr;
        // list of connected sockets and its machine information
        std::map<int, sockaddr_in> rm_connect_sockets_fd;
        ServerGroupManager *groupManager;
        ConnectionMonitor *connectionMonitor;
        int socket_fd;
        struct sockaddr_in serv_addr;
        void closeClientConnection(pair<int, int> clientConnectionId);
        std::map<string, int> connectionsCount;
        int limitOfConnectios;
        static void * monitorConnection(void *args);
        void closeFrontEndConnection(int socketId);


    public:
        Server();
        static std::vector<int> openSockets;
        Semaphore* sockets_connections_semaphore;
        static void closeClientConnection(int socket_fd);
        static void *listenFrontEndCommunication(void *newsocket);
        void setPort(int port);
        void prepareConnection();
        void printPortNumber();
        int registerUserToServer(Packet *registrationPacket, int frontEndSocket);
        int registerUser(pair<int, int> clientIdentifier, char *username, char *group,char* userID);
        int ConnectToFE();
        int handleFrontEndConnection(pthread_t *tid, pthread_t *tid2);
        void closeFrontEndConnections();
        void closeSocket();
        void closeServer();
        void init_semaphore();
        void wait_semaphore();
        void post_semaphore();
        void configureFilesystemManager(int maxNumberOfMessagesInHistory);
        int getNumberOfConnectionsByUser(string user);
        int incrementNumberOfConnectionsFromUser(string user);
        void setRmNumber(int rmNumber);
        int getRmNumber();
        void createReplicationTree();

        void createRMListenerSocket();
        static void *handleRMCommunication(void *args);
        //static void *handleConnectedRMCommunication(void *args);
        static void *acceptRMConnection(void *args);

        bool getIsPrimaryServer();
        void setIsPrimaryServer(bool value);

        void printRMConnections() const;

        void connectToRmServers();
    };
}
#endif