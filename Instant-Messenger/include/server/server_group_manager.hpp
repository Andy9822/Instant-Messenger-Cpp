#ifndef SERVER_GROUP_HPP
#define SERVER_GROUP_HPP

#include "../util/message.hpp"
#include "../util/user.hpp"
#include "../server/server_message_manager.hpp"
#include "../util/Packet.hpp"
#include "./Group.hpp"

#include <string>
#include <iostream>
#include <map> 
#include <list>
#include <vector>
#include <map>


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
        std::map<string,Group*> groupMap; // will maintain a map of groupName -> group. This will be used to route the calls to the proper group
        int addUserToGroup(User *user, string group);
        std::list<User*> getUsersByGroup(string group);
        User * getUserBySocketId(int socketId);
        void removeUserFromListOfUsers(User *user);
        void disconnectSocket(User *user, int socketId);
        bool groupExists(string groupName);

      public:
        std::list<pthread_t> *threadQueue;

        ServerGroupManager();
        int registerUserToGroup(int socket, string username, string groupName);
        void processReceivedPacket(Packet* packet);
        void disconnectUser(int socketId);
        void printListOfUsers();
        void printListOfGroups();
        void configureFileSystemManager(int maxNumberOfMessagesOnHistory);
        void sendGroupHistoryMessages(int socketId);
    };
}
#endif