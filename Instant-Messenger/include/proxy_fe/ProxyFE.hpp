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
#include "../util/Socket.hpp"
#include "../util/Semaphore.hpp"

#define MAXBACKLOG SOMAXCONN
using namespace std;

class ProxyFE : public Socket 
{
    private:
        int server_socket_fd;
        int clients_socket_fd;
        struct sockaddr_in serv_sock_addr;
        struct sockaddr_in client_sock_addr;
        pthread_t monitor_tid;
        pthread_t reconnect_server_tid;

        void processServerReconnect(void* args);   

    public:
        Semaphore* openClientsSockets_semaphore;
        std::map<int, std::pair<int, time_t>> openClientsSockets;
        Semaphore* online_semaphore;
        static bool online_RMserver;
        static int serverRM_socket;
        pthread_mutex_t mutex_server_reconnect;

        void prepareSocketConnection(int* socket_fd, sockaddr_in* serv_addr);
        void prepareServerConnection();
        void prepareClientsConnection();

        ProxyFE();
        static void closeClientConnection(int socket_fd);
        void setPortClients(int port);
        void setPortServerRM(int port);
        void prepareConnection();
        void printPortNumber();
        int handleServerConnection();
        static void* listenServerReconnect(void* args);        
        void handleServerReconnect();        
        int handleClientConnection(pthread_t *tid);
        static void* listenClientCommunication(void *args);
        static void* monitorKeepAlivesAux(void* args);        
        void monitorKeepAlives();        
};
#endif