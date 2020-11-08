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
        int socket_fd;
        int backup_socket_fd;
        bool primaryServer;
        struct sockaddr_in serv_addr;
        struct sockaddr_in backup_serv_addr;
        void closeListenClientCommunication(int client_socket);
        std::map<string, int> connectionsCount;
        static void * monitorConnection(void *args);

    public:
        Server();
        static std::vector<int> openSockets;
        static std::vector<int> replicationSockets;
        Semaphore* sockets_connections_semaphore;
        static void *listenClientCommunication(void *newsocket);
        static void closeClientConnection(int socket_fd);
        void setPort(int port);
        void setPrimaryServer(bool isPrimaryServer);
        bool getPrimaryServer();
        void prepareConnection();
        void printPortNumber();
        int registerUserToServer(void *socket);
        int registerUser(int socket, char *username, char *group);
        int handleClientConnection(pthread_t *tid);
        int connectToBackupServers();
        void closeConnections();
        void closeSocket();
        void closeServer();
        void configureFilesystemManager(int maxNumberOfMessagesInHistory);
        int incrementNumberOfConnectionsFromUser(string user);
    };
}
#endif