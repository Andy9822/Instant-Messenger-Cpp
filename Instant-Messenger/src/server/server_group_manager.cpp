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
        std::pair<int, std::string> newSocket (socket, group);

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
        for (auto const& group : groups) {
            if (group.first == groupName)
                users.push_back(group.second);
        }
        return users;
    }

    void ServerGroupManager::processReceivedPacket(Packet *packet) {
        Message receivedMessage = Message(packet->message, packet->username, packet->group, std::time(0));
        std::list<User*> users = getUsersByGroup(receivedMessage.getGroup());

        fileSystemManager->appendGroupMessageToHistory(receivedMessage);
        messageManager->broadcastMessageToUsers(receivedMessage, users);
    }
}