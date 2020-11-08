#include "../../include/server/server_group_manager.hpp"

using namespace std;

namespace servergroupmanager {

    ServerGroupManager::ServerGroupManager() : semaphore(1) {

    }

    /**
     * This function will be used to active the registration for the right groupName
     * @param socket
     * @param username
     * @param groupName
     * @return
     */
    int ServerGroupManager::registerUserToGroup(int socket, string username, string groupName) {

        // if groupName exists, send the registration to it. If it does not belong to the map of groups, we instantiate a new groupName and forward the information to it
        Group* group = NULL;
        if ( !groupExists(groupName) ) {
            this->groupMap[groupName] = new Group(groupName);
            this->groupMap[groupName]->configureFileSystemManager(this->maxNumberOfMessagesOnHistory);
        }
        group =  this->groupMap[groupName];

        return group->registerNewSession(socket, username);
    }


    /**
     * This method selects the responsible group and says: "Hey bro, here is a message. Do whatever you have to do"
     * @param packet
     */
    void ServerGroupManager::processReceivedPacket(Packet *packet) {

        if ( !groupExists(packet->group) ) {
            cout << "[ERROR] group does not exist" << endl;
            return;
        }
        this->groupMap[packet->group]->processReceivedMessage(packet->username, packet->message, packet->isBackup());
    }


    /**
     * This will the send an event to all groups saying:
     * Hey bro, this sockets disconected, it may interest you. Of so, process this event as you want.
     * @param socketId
     */
    void ServerGroupManager::propagateSocketDisconnectionEvent(int socketId, map<string, int> &numberOfConnectionsByUser) {
        std::map<basic_string<char>, Group*>::iterator it = this->groupMap.begin();

        while ( it != this->groupMap.end() ) { // iterates over the map
            Group* group = it->second;
            group->handleDisconnectEvent(socketId, numberOfConnectionsByUser);
            it++;
        }
    }


    /**
     * This sets the number of message from the history that the groups will provide to new users
     * It is used in the groups instantiation
     * @param maxNumberOfMessagesOnHistory
     */
    void ServerGroupManager::configureFileSystemManager(int maxNumberOfMessagesOnHistory) {
        this->maxNumberOfMessagesOnHistory = maxNumberOfMessagesOnHistory;
    }


    /**
     * Simply checks if the group exists or not
     * @param groupName
     * @return a boolean
     */
    bool ServerGroupManager::groupExists(string groupName) {
        if ( groupMap.find(groupName) == groupMap.end() ) {
            return false;
        }
        return true;
    }
}