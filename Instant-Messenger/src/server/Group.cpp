#include "../../include/server/Group.hpp"

using namespace std;

Group::Group(string name)
{
    this->groupName = name;
    fsManager = new filesystemmanager::FileSystemManager();
    messageManager = new servermessagemanager::ServerMessageManager();

    // Init queue semaphore
    Semaphore* sem_message_queue = new Semaphore(1);

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
                _this->messageManager->broadcastMessageToUsers(message, _this->users);
            }
        }
        
    }
    
}

void Group::saveMessageToQueue(message::Message receivedMessage) 
{
    //TODO HAS TO CHANGE TO SEMAPHORE WITH SOME KIND OF ORDER TO WAKE
    pthread_mutex_lock(&mutex_message_queue);

    messages_queue.push(receivedMessage);

    //TODO HAS TO CHANGE TO SEMAPHORE WITH SOME KIND OF ORDER TO WAKE
    pthread_mutex_unlock(&mutex_message_queue);
    pthread_mutex_unlock(&mutex_consumer_producer);
}


