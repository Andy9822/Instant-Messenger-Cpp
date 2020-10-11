#include "../../include/server/server_group_manager.hpp"

using namespace std;

namespace servergroupmanager {

    ServerGroupManager::ServerGroupManager() : semaphore(1) {
        fileSystemManager = new FileSystemManager();
        threadQueue = new std::list<pthread_t>();
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
        }
        group =  this->groupMap[groupName];

        int a = group->registerNewSession(socket, username);
        return a;
    }


    int ServerGroupManager::addUserToGroup(User *user, string userGroup)
    {
        Packet *enteringPacket;

        // Se grupo n existe, cria grupo e adiciona na lista de grupos
        if ( groupMap.find(userGroup) == groupMap.end() ) {
            Group* group = new Group(userGroup);
            groupMap[userGroup] = group;
            //TODO paramos aqui, criar grupo se nao existe e lidar c grupo existente abaixo
            
        } else {
        // found
        }

        std::list<User *> groupUsers = getUsersByGroup(userGroup);
        for (auto userItr = groupUsers.begin(); userItr != groupUsers.end(); userItr++) {
            if ((*userItr)->getUsername() == user->getUsername())
                return 1;
        }
        groups.insert(std::pair<string, User *>(userGroup, user));

        // Send <user entered the group> message
        char username[USERNAME_MAX_SIZE] = {0};
        char group[GROUP_MAX_SIZE] = {0};

        strncpy(username, user->getUsername().c_str(), USERNAME_MAX_SIZE - 1);
        strncpy(group, userGroup.c_str(), GROUP_MAX_SIZE - 1);

        enteringPacket = new Packet(username, group, (char*)"<Entered the group>", time(0));
        processReceivedPacket(enteringPacket);

        return 0;
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
        this->groupMap[packet->group]->processReceivedMessage(packet->username, packet->message);
    }


    // TODO isso provavlemente vai ter que ir pra dentro de Group
    void ServerGroupManager::disconnectUser(int socketId) {
        this->semaphore.wait();
        User *user = getUserBySocketId(socketId);
        disconnectSocket(user, socketId);

        if (user->getActiveSockets().size() == 0) {
            removeUserFromListOfUsers(user);
        }
        this->semaphore.post();

        printListOfUsers(); //debug purposes
        printListOfGroups(); //debug purposes
    }


    // TODO isso provavlemente vai ter que ir pra dentro de Group
    void ServerGroupManager::disconnectSocket(User *user, int socketId) {
//        string groupBeingDisconnected = user->getActiveSockets()->find(socketId)->second;
//        bool hasSimultaneousConnectionToRemovedGroup = false;
//
//        user->getActiveSockets()->erase(socketId);
//
//        for(auto const& socket : *user->getActiveSockets()){
//            if(socket.second == groupBeingDisconnected){ // check the removed socket group with rest of user socket list
//                hasSimultaneousConnectionToRemovedGroup = true;
//            }
//        }
//
//        if(!hasSimultaneousConnectionToRemovedGroup){
//            multimap<string, User *>::iterator userGroupEntryToBeRemoved;
//            for (auto itr = groups.begin(); itr != groups.end(); itr++) {
//                // this is responsible for fetching the correct entry o groups map
//                // so it can be removed as there are no more connections from this user to the specified group
//                if ((itr->first == groupBeingDisconnected && itr->second->getUsername() == user->getUsername())) {
//                    userGroupEntryToBeRemoved = itr;
//                }
//            }
//            groups.erase(userGroupEntryToBeRemoved);
//
//            // Send <user left the group> message
//            char username[USERNAME_MAX_SIZE] = {0};
//	        char group[GROUP_MAX_SIZE] = {0};
//
//	        strncpy(username, user->getUsername().c_str(), USERNAME_MAX_SIZE - 1);
//	        strncpy(group, groupBeingDisconnected.c_str(), GROUP_MAX_SIZE - 1);
//
//            Packet *exitingPacket = new Packet(username, group, (char*)"<Left the group>", time(0));
//			processReceivedPacket(exitingPacket);
//			delete exitingPacket;
//        }
    }



    User *ServerGroupManager::getUserBySocketId(int socketId) {
        User *user = NULL;

//        for (auto const &userItr : this->list_users) {
//            for (auto const &socket : *userItr->getActiveSockets()) {
//                if (socket.first == socketId) {
//                    user = userItr;
//                }
//            }
//        }

        return user;
    }



    void ServerGroupManager::removeUserFromListOfUsers(User *user) {
        list_users.remove(user);
    }



    void ServerGroupManager::printListOfUsers() {
        for (auto const &user: list_users) {
            cout << "\nUser: " << user->getUsername() << " with sockets: " << endl;
            user->printSockets();
        }
    }



    void ServerGroupManager::printListOfGroups() {
        cout << endl ;
        for (auto const &group: groups) {
            cout << "Group: " << group.first << " with User: " << group.second->getUsername() << endl;
        }
    }

    void ServerGroupManager::configureFileSystemManager(int maxNumberOfMessagesOnHistory) {
        this->fileSystemManager->setMaxNumberOfMessagesInHistory(maxNumberOfMessagesOnHistory);
    }

    std::list<User *> ServerGroupManager::getUsersByGroup(string groupName) {
        std::list<User *> users;
        for (auto const &group : groups) {
            if (group.first == groupName)
                users.push_back(group.second);
        }
        return users;
    }

    // ADDING THE NEW STUFF

    bool ServerGroupManager::groupExists(string groupName) {
        if ( groupMap.find(groupName) == groupMap.end() ) {
            return false;
        }
        return true;
    }
}