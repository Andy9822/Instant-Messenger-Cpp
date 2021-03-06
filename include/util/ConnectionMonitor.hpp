#ifndef INSTANT_MESSENGER_CONNECTIONKEEPER_HPP
#define INSTANT_MESSENGER_CONNECTIONMONITOR_HPP


#include <vector>
#include <map>
#include "../../include/util/Semaphore.hpp"
#include <time.h>

class ConnectionMonitor {

private:
    std::map<int, time_t> socketLastKeepAlive;
    Semaphore* socketLastKeepAliveSemaphore;
    void static * keepsMonitoringConnection(void *args); // this will be called for monitoring the connection of each socket
public:
    ConnectionMonitor();
    void monitor(int *socket);
    void refresh(int socket);
    void killSocket(int socket);
};


#endif //INSTANT_MESSENGER_CONNECTIONKEEPER_HPP
