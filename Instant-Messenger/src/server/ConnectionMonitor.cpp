#include "../../include/server/ConnectionMonitor.hpp"
#include "../../include/util/definitions.hpp"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <iostream>

#endif

using namespace std;

ConnectionMonitor::ConnectionMonitor() {
    this->socketLastKeepAliveSemaphore = new Semaphore(1);
}

void ConnectionMonitor::monitor(int *socket) {
    std::pair<int *, ConnectionMonitor *> *args = (std::pair<int *, ConnectionMonitor *> *) calloc(1, sizeof(std::pair<int *, ConnectionMonitor *>));
    args->first = socket;
    args->second = this;

    keepsMonitoringConnection(args);

    cout << "Timeout has been reached " << "\U0001F41D" << endl;

    this->socketLastKeepAliveSemaphore->wait();
    this->socketLastKeepAlive.erase(*socket);
    this->socketLastKeepAliveSemaphore->post();
}

void * ConnectionMonitor::keepsMonitoringConnection(void *args) {
    std::pair<int *, ConnectionMonitor *> *pair = (std::pair<int *, ConnectionMonitor *> *) args;
    int socket = *(int *) pair->first;
    ConnectionMonitor *_this = (ConnectionMonitor *) pair->second;

    _this->socketLastKeepAliveSemaphore->wait();
    _this->socketLastKeepAlive[socket] = time(NULL); // sets the current time
    _this->socketLastKeepAliveSemaphore->post();

    bool connectionIsValid;
    do {
        sleep(KEEP_ALIVE_INTERVAL);
        double secondsSinceLastKeepAlive;
        time_t now = time(NULL);

        _this->socketLastKeepAliveSemaphore->wait();
        secondsSinceLastKeepAlive = difftime(now, _this->socketLastKeepAlive[socket]);
        _this->socketLastKeepAliveSemaphore->post();
        connectionIsValid = secondsSinceLastKeepAlive < KEEP_ALIVE_TIMEOUT;
    } while (connectionIsValid);
}

void ConnectionMonitor::refresh(int socket) {
    this->socketLastKeepAliveSemaphore->wait();
    this->socketLastKeepAlive[socket] = time(NULL); // sets the current time
    this->socketLastKeepAliveSemaphore->post();
}

void ConnectionMonitor::killSocket(int socket) {
    this->socketLastKeepAliveSemaphore->wait();
    this->socketLastKeepAlive.erase(socket);
    this->socketLastKeepAliveSemaphore->post();
}
