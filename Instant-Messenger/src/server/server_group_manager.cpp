#include "../../include/server/server_group_manager.hpp"

using namespace std;

namespace servergroupmanager {

    ServerGroupManager::ServerGroupManager() : semaphore(1) {
        fileSystemManager = new FileSystemManager();
    }

    int ServerGroupManager::registerUserToGroup(int socket, string username, string group) {
        this->semaphore.wait();
        bool userAlreadyExists = false;
        User *newUser;
        list<User *>::iterator user;
        int result = 0;
        std::pair<int, std::string> newSocket(socket, group);

        for (user = list_users.begin(); user != list_users.end(); ++user) {
            if ((*user)->getUsername() == username) {
                if ((*user)->registerSession(newSocket) < 0) {
                    result = -1;
                    break;
                }

                addUserToGroup(*user, group);
                userAlreadyExists = true;

                break;
            }
        }

        this->semaphore.post();

        if (!userAlreadyExists) {
            this->semaphore.wait();
            newUser = new User(username);

            if (newUser->registerSession(newSocket) < 0)
                result = -1;

            addUserToGroup(newUser, group);
            list_users.push_back(newUser);
            this->semaphore.post();
        }

        return result;
    }

    void ServerGroupManager::addUserToGroup(User *user, string userGroup) {
        std::list<User *> groupUsers = getUsersByGroup(userGroup);
        for (auto userItr = groupUsers.begin(); userItr != groupUsers.end(); userItr++) {
            if ((*userItr)->getUsername() == user->getUsername())
                return;
        }
        groups.insert(std::pair<string, User *>(userGroup, user));
    }

    std::list<User *> ServerGroupManager::getUsersByGroup(string groupName) {
        std::list<User *> users;
        for (auto const &group : groups) {
            if (group.first == groupName)
                users.push_back(group.second);
        }
        return users;
    }

    void ServerGroupManager::processReceivedPacket(Packet *packet) {
        Message receivedMessage = Message(packet->message, packet->username, packet->group, std::time(0));
        std::list<User *> users = getUsersByGroup(receivedMessage.getGroup());

        fileSystemManager->appendGroupMessageToHistory(receivedMessage);
        messageManager->broadcastMessageToUsers(receivedMessage, users);
    }

    void ServerGroupManager::disconnectUser(int socketId) {
        this->semaphore.wait();
        User *user = getUserBySocketId(socketId);
        disconnectSocket(user, socketId);

        if (user->getActiveSockets()->size() == 0) {
            removeUserFromListOfUsers(user);
        }
        this->semaphore.post();

        printListOfUsers(); //debug purposes
        printListOfGroups(); //debug purposes
    }

    void ServerGroupManager::disconnectSocket(User *user, int socketId) {
        string groupBeingDisconnected = user->getActiveSockets()->find(socketId)->second;
        bool hasSimultaneousConnectionToRemovedGroup = false;

        user->getActiveSockets()->erase(socketId);

        for(auto const& socket : *user->getActiveSockets()){
            if(socket.second == groupBeingDisconnected){ // check the removed socket group with rest of user socket list
                hasSimultaneousConnectionToRemovedGroup = true;
            }
        }

        if(!hasSimultaneousConnectionToRemovedGroup){
            multimap<string, User *>::iterator userGroupEntryToBeRemoved;
            for (auto itr = groups.begin(); itr != groups.end(); itr++) {
                // this is responsible for fetching the correct entry o groups map
                // so it can be removed as there are no more connections from this user to the specified group
                if ((itr->first == groupBeingDisconnected && itr->second->getUsername() == user->getUsername())) {
                    userGroupEntryToBeRemoved = itr;
                }
            }

            groups.erase(userGroupEntryToBeRemoved);
        }
    }

    User *ServerGroupManager::getUserBySocketId(int socketId) {
        User *user = NULL;

        for (auto const &userItr : this->list_users) {
            for (auto const &socket : *userItr->getActiveSockets()) {
                if (socket.first == socketId) {
                    user = userItr;
                }
            }
        }

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
}