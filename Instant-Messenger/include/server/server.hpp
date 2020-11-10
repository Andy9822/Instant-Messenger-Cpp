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
        bool isPrimaryServer;
        int socket_fd;

        //todo: review it
        std::vector<pair<int, struct sockaddr_in>> backupConnections;
        int backup_socket_fd;
        struct sockaddr_in backup_serv_addr;

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
        void setPort(int fePort);
        void prepareConnection();
        void printPortNumber();
        int registerUserToServer(Packet *registrationPacket, int frontEndSocket);
        int registerUser(pair<int, int> clientIdentifier, char *username, char *group,char* userID);
        int ConnectToFE();
        //int connectToPrimaryServer();
        int prepareBackupServersConnection();
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
        bool getIsPrimaryServer();
        void setIsPrimaryServer(bool isPrimaryServer);
        bool shouldSendReplicationPackage(Packet *receivedPacket);

        void setOutboundPort(int port);
        void setInboundPort(int port);
    };
}
#endif