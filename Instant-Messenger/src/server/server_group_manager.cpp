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

        for (user = list_users.begin(); user != list_users.end(); ++user) {
            if ((*user)->getUsername() == username) {
                if ((*user)->registerSession(socket) < 0) {
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

            if (newUser->registerSession(socket) < 0)
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
        group.insert(std::pair<string, User *>(userGroup, user));
    }

    std::list<User *> ServerGroupManager::getUsersByGroup(string groupName) {
        std::list<User *> users;
        for (auto itr = group.begin(); itr != group.end(); itr++) {
            if (itr->first == groupName)
                users.push_back(itr->second);
            //cout << itr -> first << "  " << itr -> second->getUsername() << endl;	// TODO: REMOVE
        }
        return users;
    }

    void ServerGroupManager::processReceivedPacket(Packet *packet) {
        Message receivedMessage = Message(packet->message, packet->username, packet->group, std::time(0));
        std::list<User*> users = getUsersByGroup(receivedMessage.getGroup());

        //cout << "debug entering appendGroupMessageToHistory \n";
        fileSystemManager->appendGroupMessageToHistory(receivedMessage);
        //cout << "debug entering broadcastMessageToUsers \n";
        messageManager->broadcastMessageToUsers(receivedMessage, users);
    }
}