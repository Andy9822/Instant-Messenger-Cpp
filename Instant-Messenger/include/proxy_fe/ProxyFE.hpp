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
        int socket_fd;
        struct sockaddr_in serv_addr;
        pthread_t monitor_tid;

    public:
        Semaphore* openClientsSockets_semaphore;
        std::map<int, std::pair<int, time_t>> openClientsSockets;
        Semaphore* online_semaphore;
        static bool online_RMserver;
        static int serverRM_socket;
        ProxyFE();
        static void closeClientConnection(int socket_fd);
        void setPort(int port);
        void prepareConnection();
        void printPortNumber();
        int handleServerConnection();        
        int handleClientConnection(pthread_t *tid);
        static void* monitorKeepAlivesAux(void* args);        
        void monitorKeepAlives();        
};
#endif