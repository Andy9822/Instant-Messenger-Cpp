#ifndef REPLICATION_HPP
#define REPLICATION_HPP

#include <map>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../util/Socket.hpp"
#include "../util/Semaphore.hpp"
#include "../util/definitions.hpp"
#include "../server/server_group_manager.hpp"
#include <sys/socket.h>
#define MAXBACKLOG SOMAXCONN
//#include "../../include/server/server.hpp"

using namespace std;
using namespace servergroupmanager;

class ReplicationManager : public Socket{
    public:
        ReplicationManager(ServerGroupManager *groupManager);
        //ReplicationManager(ServerGroupManager *groupManager);
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
        ServerGroupManager *groupManager;

    private:

};

#endif