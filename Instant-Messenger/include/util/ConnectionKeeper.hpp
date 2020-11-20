#ifndef INSTANT_MESSENGER_CONNECTIONKEEPER_HPP
#define INSTANT_MESSENGER_CONNECTIONKEEPER_HPP


#include "../util/Socket.hpp"
#include "../util/definitions.hpp"
#include <vector>

class ConnectionKeeper : public Socket{

public:
    ConnectionKeeper(int socket);
    ConnectionKeeper(vector<int> socket);
private:
    int sleepTime = KEEP_ALIVE_INTERVAL;
    int communicationSocket;
    void startSendingKeepAlive();
    static void * sendKeepAliveForever(void *args);

};


#endif //INSTANT_MESSENGER_CONNECTIONKEEPER_HPP
