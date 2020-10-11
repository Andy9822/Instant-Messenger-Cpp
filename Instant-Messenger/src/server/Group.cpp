#include "../../include/server/Group.hpp"
#include <algorithm>

using namespace std;

Group::Group(string name)
{
    this->groupName = name;
    fsManager = new filesystemmanager::FileSystemManager();
    messageManager = new servermessagemanager::ServerMessageManager();

    // Init queue semaphore
    sem_message_queue = new Semaphore(1);

    // Init consumer/producer mutex
    pthread_mutex_init(&mutex_consumer_producer, NULL);

    // Consumer/producer mutex init locked, due to absence of messages in the queue to be consumed
    pthread_mutex_lock(&mutex_consumer_producer);

    // Create thread 24/7 alive consuming from queue when available
    pthread_create(&tid, NULL, consumeMessageQueue, (void *) this);
}

void * Group::consumeMessageQueue(void * args)
{
    Group* _this = (Group *) args;
    while (true)
    {
        pthread_mutex_lock(&(_this->mutex_consumer_producer));

        bool hasMessagesInQueue = true;
        while (hasMessagesInQueue)
        {
            
            _this->sem_message_queue->wait();

            if (_this->messages_queue.empty())
            {
                hasMessagesInQueue = false;
                _this->sem_message_queue->post();
            }
            else
            {
                message::Message message = _this->messages_queue.front();
                _this->messages_queue.pop();

                _this->sem_message_queue->post();


                _this->fsManager->appendGroupMessageToHistory(message);
                _this->messageManager->broadcastMessageToUsers(message, _this->getAllActiveSockets());
            }
        }
        
    }
    
}

/**
 * This method will register the user in the group by adding it to the users list and adding the reference to the socket
 * If user already exists in the list, we just add the id for the socket in the connections
 * at this point, we dont need to care about persisting more connections for users since the number of connections for users
 * is maintained by the server.
 *
 * After all, it calls the method responsible for notify all other users in the same group
 *
 * @param socket
 * @param userName
 * @param groupName
 * @return returns a negative number in case of a failure
 */
int Group::registerNewSession(int socket, string userName) {
    User* user = NULL;
    int result = 0;


    sendHistoryToUser(socket);
    for (auto userItr : this->users) {
        if ( userName.compare(userItr->getUsername()) == 0 ) {
            user = userItr;
            break;
        }
    }
    if ( user == NULL) { // if user does not exists in the list, we create the entry in the list
        user = new User(userName);
        this->users.push_back(user);
        result = user->registerSession(socket);
        sendActivityMessage(userName, "<Joined the group>"); // Se a pessoa já está no grupo, não deve-se enviar uma nova mensagem dizendo que ela ingressou no grupo. (copiei do moodle esse statement)
    } else {
        result = user->registerSession(socket);
    }
    return result;
}

/**
 * This function handles the disconnect events from the upper layer
 * @param socket
 */
void Group::handleDisconnectEvent(int socket, map<string, int> &numberOfConnectionsByUser) {
    vector<int> allActiveSockets = this->getAllActiveSockets();
    if (std::count(allActiveSockets.begin(), allActiveSockets.end(), socket)) { // element found
        this->disconnectSession(socket, numberOfConnectionsByUser);
    }
}


/**
 * This function will terminated a user connection
 * Not a big deal, we just need to pay attention to two cases:
 *  1. will the system keep another connection open?
 *      if yes - we just kill the connection
 *      if not - we remove the user from the user's list and send a notification
 * @param socketId
 */
void Group::disconnectSession(int socketId, map<string, int> &numberOfConnectionsByUser) {
    user::User* user = getUserFromSocket(socketId);
    if ( user != NULL) {
        numberOfConnectionsByUser[user->getUsername()] -= 1;
        user->releaseSession(socketId);
        if (user->getActiveSockets().size() < 1) {
            sendActivityMessage(user->getUsername(), "<Left the group>");
            removeUserFromGroup(user);
        }
    }
}


/**
 * Send a notification to the group (entered or join)
 * @param userName
 */
void Group::sendActivityMessage(const string &userName, const string &actionText) {
    Message entryNotificationMessage = Message(actionText, userName, groupName, time(0));
    entryNotificationMessage.setIsNotification(true);
    addMessageToMessageQueue(entryNotificationMessage);
}


/**
 * This little friend can send the history to a socket
 * It can help you to welcome new users and introduce them to the discussed topics
 * @param socketId
 */
void Group::sendHistoryToUser(int socketId) {
    std::vector<Message> messages = fsManager->readGroupHistoryMessages(this->groupName);
    sem_message_queue->wait();
    for(auto  message : messages) {
        message.setIsNotification(true);
        messageManager->sendMessageToSocketId(message, socketId);
    }
    sem_message_queue->post();
}


/**
 * API for the group manager used to process the new message in the  group level
 * @param userName
 * @param message
 */
void Group::processReceivedMessage(string userName, string message) {
    Message receivedMessage = Message(message, userName, this->groupName, std::time(0));
    addMessageToMessageQueue(receivedMessage);
}


void Group::removeUserFromGroup(User *user) {
    users.remove(user);
}

/**
 * Auxiliar method to get the user by the socket
 * @param socketId
 * @return
 */
User *Group::getUserFromSocket(int socketId) const {
    for (auto user : users) {
        for (auto socket : user->getActiveSockets()) {
            if (socket == socketId) {
                return user;
            }
        }
    }
    return NULL;
}

/**
 * Method responsible for adding the message to the message queue
 * @param userName
 * @param message
 */
void Group::addMessageToMessageQueue(Message message) {
    sem_message_queue->wait();
    messages_queue.push(message);
    sem_message_queue->post();
    pthread_mutex_unlock(&mutex_consumer_producer);
}

/**
 * Can you guess what this method does?
 * @return list of sockets
 */
vector<int> Group::getAllActiveSockets() {
    vector<int> sockets = vector<int>();
    for (auto user : this->users) {
        for (auto userActiveSocket : user->getActiveSockets()) {
            cout << "[DEBUG] Group::getAllActiveSockets - user: " << user->getUsername() << " socket: " << userActiveSocket << endl;
            sockets.push_back(userActiveSocket);
        }
    }
    return sockets;
}

void Group::configureFileSystemManager(int maxNumberOfMessagesOnHistory) {
    this->fsManager->setMaxNumberOfMessagesInHistory(maxNumberOfMessagesOnHistory);
}



