#include "../../include/server/Group.hpp"
#include "../../include/util/StringConstants.hpp"
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
 * This function sends an ACCEPT packet to the user saying that
 * he was able to join the group
 *
 * TODO: maybe this responsibility should not be in the group. Analyse a better place to place it, maybe on server. This has nothing to do with the group
 *
 * @param clientID
 * @param feSocket
 */
void Group::sendAcceptToUser(char *clientID, int feSocket)
{
    Packet *pack = new Packet();
    pack->type = ACCEPT_PACKET;
    strcpy(pack->user_id, clientID);
    std::cout << "[DEBUG] mandei ACCEPT para socket: " << feSocket << std::endl;
    messageManager->sendPacketToSocketId(pack, feSocket);
}

/**
 * This method will register the user in the group by adding it to the users list and adding the reference to the socket
 * If user already exists in the list, we just add the id for the socket in the connections
 * at this point, we dont need to care about persisting more connections for users since the number of connections for users
 * is maintained by the server.
 *
 * After all, it calls the method responsible for notify all other users in the same group
 *
 * @param feSocket
 * @param userName
 * @param groupName
 * @return returns a negative number in case of a failure
 */
int Group::registerNewSession(char *clientID, int feSocket, string userName) {
    User* user = NULL;
    int result = 0;
    sendAcceptToUser(clientID, feSocket);
    sendHistoryToUser(clientID, feSocket);
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
        result = user->registerSession(clientID, feSocket);
        sendActivityMessage(userName, JOINED_MESSAGE); // Se a pessoa já está no grupo, não deve-se enviar uma nova mensagem dizendo que ela ingressou no grupo. (copiei do moodle esse statement)
    } else {
        result = user->registerSession(clientID, feSocket);
    }
    usersSemaphore->post();
    return result;
}

/**
 * This method handles the disconnect events from the upper layer
 *
 * @param feSocket
 */
void Group::handleDisconnectEvent(char *clientID, int feSocket, map<string, int> &numberOfConnectionsByUser) {
    usersSemaphore->wait();
    vector<pair <char *, int> > allActiveSockets = this->getAllActiveConnectionIds();

    if ( strcmp(clientID, FE_DISCONNECT) == 0 ) { // DELETE ALL CONNECTIONS FROM THE CLIENTS THAT WERE CONNECTED TO THE FE
        for (auto groupConnection : allActiveSockets) {
            if (groupConnection.second == feSocket) { // if there is a match in the FE socket ID
                cout << "Killing clientConnection [" << groupConnection.first << "," << groupConnection.second << "]" << endl;
                this->disconnectSession(groupConnection.first, feSocket, numberOfConnectionsByUser);
            }
        }
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
 * @param feSocket
 */
void Group::disconnectSession(char *clientID, int feSocket, map<string, int> &numberOfConnectionsByUser) {
    user::User* user = getUserFromConnectionId(clientID, feSocket);
    cout << "disconnectSession  [" << clientID << "," << feSocket << "]" << endl;
    if ( user != NULL) {
        numberOfConnectionsByUser[user->getUsername()] -= 1;
        user->releaseSession(clientID, feSocket);
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
 * @param feSocket
 */
void Group::sendHistoryToUser(char *clientID, int feSocket) {
    std::vector<Message> messages = fsManager->readGroupHistoryMessages(this->groupName);
    cout << "Vou printar as mensagens do user " << endl;
    messageQueueSemaphore->wait();
    for(auto  message : messages) {
        message.setIsNotification(true);
        messageManager->sendMessageToSocketId(message, clientID, feSocket);
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
 * @param feSocket
 * @return
 */
User *Group::getUserFromConnectionId(char *clientID, int feSocket) const {
    for (auto user : users) {
        for (auto userConnection : user->getActiveConnections()) {
            if ( ( strcmp(clientID, userConnection.first) == 0) && ( feSocket == userConnection.second ) )  {
                return user;
            }
        }
    }
    return nullptr;
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
vector<pair<char *, int>> Group::getAllActiveConnectionIds() {
    vector< pair <char *, int> > connectionIds = vector< pair<char *, int> >();
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



