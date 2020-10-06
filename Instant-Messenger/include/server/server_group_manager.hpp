#ifndef SERVER_GROUP_HPP
#define SERVER_GROUP_HPP

#include "../util/message.hpp"
#include "../util/user.hpp"
#include "../util/file_system_manager.hpp"
#include "../server/server_message_manager.hpp"
#include "../util/Packet.hpp"

#include <string>
#include <iostream>
#include <map> 
#include <list>
#include <vector>

using namespace std;
using namespace user;
using namespace message;
using namespace filesystemmanager;
using namespace servermessagemanager;

namespace servergroupmanager {

    class ServerGroupManager {

        private:
        ServerMessageManager *messageManager;
        FileSystemManager *fileSystemManager;
        Semaphore semaphore;
        list<User*> list_users;
        multimap<string, User*> groups;
        void addUserToGroup(User *user, string group);
        std::list<User*> getUsersByGroup(string group);

      public:
        ServerGroupManager();
        int registerUserToGroup(int socket, string username, string groupName);
        void processReceivedPacket(Packet* packet);
    };
}
#endif