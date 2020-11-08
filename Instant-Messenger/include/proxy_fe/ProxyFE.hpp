#ifndef PROXYFE_HPP
#define PROXYFE_HPP

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
#include <ctime>
#include <utility>
#include <map>
#include "../util/Packet.hpp"
#include "../util/Semaphore.hpp"
#include "../util/ConnectionMonitor.hpp"
#include "../util/Socket.hpp"

#define MAXBACKLOG SOMAXCONN
using namespace std;

class ProxyFE : public Socket
{
    private:
        int connections_server_socket_fd;
        int clients_socket_fd;
        struct sockaddr_in serv_sock_addr;
        struct sockaddr_in client_sock_addr;
        pthread_t monitor_tid;
        pthread_t reconnect_server_tid;

        void processServerReconnect(void* args);   

    public:
        Semaphore* openClientsSockets_semaphore;
        std::map<int, std::pair<pthread_t, time_t>> openClientsSockets;
        std::map<string, int> usersMap;
        static Semaphore* users_map_semaphore;
        std::map<int, string> socketsMap;
        static Semaphore* sockets_map_semaphore;
        static Semaphore* online_semaphore;
        Semaphore* processing_message_semaphore;
        Packet* processing_message;
        static bool online_RMserver;
        static int serverRM_socket;
        pthread_mutex_t mutex_server_reconnect;
        pthread_mutex_t mutex_consumer_message;
        ConnectionMonitor* keepAliveMonitor;

        void prepareSocketConnection(int* socket_fd, sockaddr_in* serv_addr);
        void prepareServerConnection();
        void prepareClientsConnection();
        void handleSocketDisconnection(int socket);
        void handleServerDisconnection(int socket);

        void processClientPacket(Packet* receivedPacket, int socket);
        void processServerPacket(Packet* receivedPacket, int socket);

        ProxyFE();
        static void closeClientConnection(int socket_fd);
        void setPortClients(int port);
        void setPortServerRM(int port);
        void prepareConnection();
        void printPortNumber();
        int handleServerConnection(pthread_t *tid);
        static void* listenServerReconnect(void* args);        
        void handleServerReconnect(pthread_t *tid);        
        int handleClientConnection(pthread_t *tid);

        void registerUserSocket(Packet* receivedPacket, int socket);
        static void* listenClientCommunication(void *args);
        static void* listenServerCommunication(void *args);

        static void* monitorConnectionKeepAlive(void *args);

        static void* handleProcessingMessage(void* args);
        void activateMessageConsumer(pthread_t *tid);     
        void processIncomingClientMessage(Packet* message);     
};
#endif