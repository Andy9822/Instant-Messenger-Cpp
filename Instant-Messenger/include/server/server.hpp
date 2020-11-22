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
#include "../util/ConnectionMonitor.hpp"
#include "FeAddressBook.hpp"


#define MAXBACKLOG SOMAXCONN

using namespace servergroupmanager;

namespace server {
    class Server : public Socket {
    private:

        std::map<int, sockaddr_in> rm_connect_sockets_fd;
        ServerGroupManager *groupManager;
        ConnectionMonitor *connectionMonitor;

        struct sockaddr_in serv_addr;
        void closeClientConnection(pair<string, string> clientConnectionId);
        std::map<string, int> connectionsCount;
        int limitOfConnectios;
        static void * monitorConnection(void *args);
        void closeFrontEndConnection(string feAddress);
        pthread_t tid[MAXBACKLOG];
        FeAddressBook* feAddressBook;
        vector<string> feAddresses;

    public:
        Server();
        vector<int> socketFeList;
        static std::vector<int> openSockets;
        Semaphore* sockets_connections_semaphore;
        Semaphore* feConnectionInitializationSemaphore;
        Semaphore* feAddressesSemaphore;
        static void closeClientConnection(int socket_fd);
        static void *listenFrontEndCommunication(void *newsocket);
        void setPort(int port);
        void prepareConnection();
        void printPortNumber();
        int registerUserToServer(Packet *registrationPacket, string feAddress);
        int registerUser(pair<string, string> clientIdentifier, char *username, char *group);
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

        void purgeFeConnection(string feAddress);
        int getSocketFromAddress(const string feAddress);
        void eraseSocketFromFeSocketList(int socketId);

        static bool isPrimaryServer;
        static bool getIsPrimaryServer;
        void prepareReplicationManager(int rmNumber);

        void connectToRmServers();
        void createRMListenerSocket();
        static void *handleRMCommunication(void *args);
        static void *acceptRMConnection(void *args);
        int rmNumber;
        int rm_listening_socket_fd;
        struct sockaddr_in rm_listening_serv_addr;
        void printRMConnections() const;
        void setRmNumber(int rmNumber);
        int getRmNumber();
        void sendMockDataToRMServers();
    };
}
#endif