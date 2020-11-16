#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <string>
#include "../../include/util/Packet.hpp"
#include "../../include/util/ConnectionKeeper.hpp"


ConnectionKeeper::ConnectionKeeper(int socket) {
    pthread_t senderThread;
    this->communicationSocket = socket;
    pthread_create(&senderThread, NULL, sendKeepAliveForever, (void*) this);
}

void * ConnectionKeeper::sendKeepAliveForever(void *args) {
    ConnectionKeeper *_this = (ConnectionKeeper *) args;

    Packet *keepAlivePacket = new Packet(KEEP_ALIVE_PACKET);
    while (true)
    {
        sleep(_this->sleepTime);
        _this->sendPacket(_this->communicationSocket, keepAlivePacket);
    }

    return NULL;
}


