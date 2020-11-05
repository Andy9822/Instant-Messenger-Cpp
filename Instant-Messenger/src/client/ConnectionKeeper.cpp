//
// Created by gabriel on 11/4/20.
//

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <string>
#include "../../include/util/Packet.hpp"
#include "../../include/client/ConnectionKeeper.hpp"


ConnectionKeeper::ConnectionKeeper(int socket) {
    pthread_t monitoringThread;
    this->communicationSocket = socket;
    pthread_create(&monitoringThread, NULL, sendKeepAliveForever, (void*) this);
}

void * ConnectionKeeper::sendKeepAliveForever(void *args) {
    ConnectionKeeper *_this = (ConnectionKeeper *) args;

    Packet *keepAlivePacket = new Packet();
    while (true)
    {
        sleep(_this->sleepTime);
        keepAlivePacket->isKeepAlive = true;
        _this->sendPacket(_this->communicationSocket, keepAlivePacket);
    }
}


