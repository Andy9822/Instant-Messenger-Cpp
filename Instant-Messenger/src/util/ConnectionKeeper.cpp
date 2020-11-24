#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <string>
#include "../../include/util/Packet.hpp"
#include "../../include/util/ConnectionKeeper.hpp"


ConnectionKeeper::ConnectionKeeper(int socket) {
    cout << "Starts sending keep alive to socket" << socket << endl;
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
        if (_this->sendPacket(_this->communicationSocket, keepAlivePacket) == 0) {
            cout << "Stop sending keep alives to socket " << _this->communicationSocket << " because we lost it :(" <<  endl;
            break;
        };
    }

    return NULL;
}


