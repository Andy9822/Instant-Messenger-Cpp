#ifndef SERVER_GROUP_HPP
#define SERVER_GROUP_HPP

#include "../util/message.hpp"
#include "../util/user.hpp"
#include "../util/Socket.hpp"
#include "../server/server_message_manager.hpp"
#include "../util/Packet.hpp"
#include "./Group.hpp"
#include "FeAddressBook.hpp"

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

    class ServerGroupManager : Socket {
        private:
            Semaphore semaphore;
            std::map<string,Group*> groupMap; // will maintain a map of groupName -> group. This will be used to route the calls to the proper group
            bool groupExists(string groupName);
            int maxNumberOfMessagesOnHistory;
            FeAddressBook feAddressBook;

        public:
            ServerGroupManager(FeAddressBook feAddressBook);
            int registerUserToGroup(pair<string, string> clientIdentifier, string username, string groupName);
            void processReceivedPacket(Packet* packet);
            void propagateSocketDisconnectionEvent(pair<string, string> connectionId, map<string, int> &numberOfConnectionsByUser);
            void configureFileSystemManager(int maxNumberOfMessagesOnHistory);

    };
}
#endif