#ifndef INSTANT_MESSENGER_CONNECTIONKEEPER_HPP
#define INSTANT_MESSENGER_CONNECTIONKEEPER_HPP


#include "../util/Socket.hpp"
#include "../util/definitions.hpp"

class ConnectionKeeper : public Socket{

public:
    ConnectionKeeper(int socket);

private:
    int sleepTime = KEEP_ALIVE_INTERVAL;
    int communicationSocket;
    static void * sendKeepAliveForever(void *args);

};


#endif //INSTANT_MESSENGER_CONNECTIONKEEPER_HPP
