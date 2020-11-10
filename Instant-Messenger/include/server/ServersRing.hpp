#ifndef SERVERS_RING_HPP
#define SERVERS_RING_HPP

#include "../../include/server/server.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include <string>

using namespace std;

using namespace server;

namespace servers_ring {
    class ServersRing : public Socket {
    private:
        int socket_client, socket_server;
        struct sockaddr_in serv_addr;
        struct sockaddr_in client_addr;
        bool disconnected;
        ifstream myfile;

    public:
        ServersRing();
        int getNewPortFromFile();
        void connectServersRing(Server serverApp);
        void prepareServerConnection();
        void prepareClientConnection(char *ip_adress);
        static void *connectToServer(void * args);
        static void *AcceptServerConnection(void * args);
        static void *listenClientCommunication(void *args); 
        void checkIfConnectionFailed();
    };
}

#endif