#ifndef GROUP_HPP
#define GROUP_HPP
#include <vector>
#include <list>
#include <iostream>
#include <pthread.h>
#include <queue> 
#include "../util/user.hpp"
#include "../util/message.hpp"
#include "../util/Semaphore.hpp"
#include "../util/file_system_manager.hpp"
#include "../server/server_message_manager.hpp"

using namespace std;
class Group
{

	public:
		Group();
        Group(string groupName);
        filesystemmanager::FileSystemManager* fsManager;
        servermessagemanager::ServerMessageManager *messageManager;
        pthread_t tid;
        list<user::User*> users;
        pthread_mutex_t mutex_consumer_producer;
        Semaphore* messageQueueSemaphore;
        Semaphore* usersSemaphore;
        std::queue<message::Message> messages_queue;  
        string groupName;
        static void *consumeMessageQueue(void * args);
        int registerNewSession(int socket, string username);
        void processReceivedMessage(string userName, string message);
        void handleDisconnectEvent(int socket, map<string, int> &numberOfConnectionsByUser);
        void configureFileSystemManager(int maxNumberOfMessagesOnHistory);

    private:
        vector<int> getAllActiveSockets();
        void sendHistoryToUser(int socketId);
        void addMessageToMessageQueue(Message message);
        void sendActivityMessage(const string &userName, const string &actionText);
        void disconnectSession(int socketId, map<string, int> &numberOfConnectionsByUser);
        User *getUserFromSocket(int socketId) const;
        void removeUserFromGroup(User *user);

};

#endif