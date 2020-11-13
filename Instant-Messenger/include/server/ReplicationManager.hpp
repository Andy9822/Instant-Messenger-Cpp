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

using namespace std;
using namespace servergroupmanager;

class ReplicationManager : public Socket{
    public:


    private:

};

#endif