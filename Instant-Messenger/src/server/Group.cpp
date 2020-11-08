#include "../../include/server/Group.hpp"
#include <algorithm>

using namespace std;

Group::Group(string name)
{
    this->groupName = name;
    fsManager = new filesystemmanager::FileSystemManager();
    messageManager = new servermessagemanager::ServerMessageManager();

    // Init semaphores
    messageQueueSemaphore = new Semaphore(1);
    usersSemaphore = new Semaphore(1);

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
            
            _this->messageQueueSemaphore->wait();

            if (_this->messages_queue.empty())
            {
                hasMessagesInQueue = false;
                _this->messageQueueSemaphore->post();
            }
            else
            {
                message::Message message = _this->messages_queue.front();
                _this->messages_queue.pop();

                _this->messageQueueSemaphore->post();


                _this->fsManager->appendGroupMessageToHistory(message);
                _this->usersSemaphore->wait();
                _this->messageManager->broadcastMessageToUsers(message, _this->getAllActiveConnectionIds());
                _this->usersSemaphore->post();
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
 * @param clientIdentifier
 * @param userName
 * @param groupName
 * @return returns a negative number in case of a failure
 */
int Group::registerNewSession(pair<int, int> clientIdentifier, string userName) {
    User* user = NULL;
    int result = 0;

    sendHistoryToUser(clientIdentifier);
    usersSemaphore->wait();
    for (auto userItr : this->users) {
        if ( userName.compare(userItr->getUsername()) == 0 ) {
            user = userItr;
            break;
        }
    }
    if ( user == NULL) { // if user does not exists in the list, we create the entry in the list
        user = new User(userName);
        this->users.push_back(user);
        result = user->registerSession(clientIdentifier);
        sendActivityMessage(userName, JOINED_MESSAGE); // Se a pessoa já está no grupo, não deve-se enviar uma nova mensagem dizendo que ela ingressou no grupo. (copiei do moodle esse statement)
    } else {
        result = user->registerSession(clientIdentifier);
    }
    usersSemaphore->post();
    return result;
}

/**
 * This function handles the disconnect events from the upper layer
 * @param connectionId
 */
void Group::handleDisconnectEvent(pair<int, int> connectionId, map<string, int> &numberOfConnectionsByUser) {
    usersSemaphore->wait();
    vector<pair <int, int> > allActiveSockets = this->getAllActiveConnectionIds();

    if (connectionId.first == FE_DISCONNECT) { // DELETE ALL CONNECTIONS FROM THE CLIENTS THAT WERE CONNECTED TO THE FE
        for (auto groupConnection : allActiveSockets) {
            if (groupConnection.second == connectionId.second) { // if there is a match in the FE socket ID
                cout << "Killing clientConnection [" << groupConnection.first << "," << groupConnection.second << "]" << endl;
                this->disconnectSession(groupConnection, numberOfConnectionsByUser);
            }
        }
    } else if (std::count(allActiveSockets.begin(), allActiveSockets.end(), connectionId)) { // element found
        this->disconnectSession(connectionId, numberOfConnectionsByUser);
    }

    usersSemaphore->post();
}


/**
 * This function will terminated a user connection
 * Not a big deal, we just need to pay attention to two cases:
 *  1. will the system keep another connection open?
 *      if yes - we just kill the connection
 *      if not - we remove the user from the user's list and send a notification
 *  it is already thread safe by the call (handleDisconnectEvent)
 * @param connectionId
 */
void Group::disconnectSession(pair<int, int> connectionId, map<string, int> &numberOfConnectionsByUser) {
    user::User* user = getUserFromConnectionId(connectionId);
    if ( user != NULL) {
        numberOfConnectionsByUser[user->getUsername()] -= 1;
        user->releaseSession(connectionId);
        if (user->getActiveConnections().size() < 1) {
            sendActivityMessage(user->getUsername(), LEFT_GROUP_MESSAGE);
            users.remove(user);
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
 * @param clientIdentifier
 */
void Group::sendHistoryToUser(pair<int, int> clientIdentifier) {
    std::vector<Message> messages = fsManager->readGroupHistoryMessages(this->groupName);
    messageQueueSemaphore->wait();
    for(auto  message : messages) {
        message.setIsNotification(true);
        messageManager->sendMessageToSocketId(message, clientIdentifier);
    }
    messageQueueSemaphore->post();
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

/**
 * Auxiliary method to get the user by the connection
 * @param connectionId
 * @return
 */
User *Group::getUserFromConnectionId(pair<int, int> connectionId) const {
    for (auto user : users) {
        for (auto groupConnectionId : user->getActiveConnections()) {
            if (groupConnectionId == connectionId) {
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
    messageQueueSemaphore->wait();
    messages_queue.push(message);
    messageQueueSemaphore->post();
    pthread_mutex_unlock(&mutex_consumer_producer);
}

/**
 * Can you guess what this method does?
 * @return list of sockets
 */
vector<pair<int, int>> Group::getAllActiveConnectionIds() {
    vector< pair <int, int> > connectionIds = vector< pair<int, int> >();
    for (auto user : this->users) {
        for (auto userActiveSocket : user->getActiveConnections()) {
            connectionIds.push_back(userActiveSocket);
        }
    }
    return connectionIds;
}

void Group::configureFileSystemManager(int maxNumberOfMessagesOnHistory) {
    this->fsManager->setMaxNumberOfMessagesInHistory(maxNumberOfMessagesOnHistory);
}



