#ifndef REPLICATION_HPP
#define REPLICATION_HPP

#include <map>
#include "../util/Socket.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../util/Semaphore.hpp"

using namespace std;

class ReplicationManager : public Socket{

    public:
        ReplicationManager();
        void connectToRmServers();
        std::map<int, sockaddr_in> rm_connect_sockets_fd;
        void createRMListenerSocket();
        static void *handleRMCommunication(void *args);
        static void *acceptRMConnection(void *args);
        int rmNumber;
        int rm_listening_socket_fd;
        struct sockaddr_in rm_listening_serv_addr;
        void printRMConnections() const;
        Semaphore* sockets_connections_semaphore;
        void setRmNumber(int rmNumber);
        int getRmNumber();
        void sendMockDataToRMServers();

    private:

};

#endif