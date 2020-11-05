//
// Created by gabriel on 11/4/20.
//

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
    this->socketMapSemaphore = new Semaphore(1);
}

void ConnectionMonitor::monitor(int *socket) {
    std::pair<int *, ConnectionMonitor *> *args = (std::pair<int *, ConnectionMonitor *> *) calloc(1, sizeof(std::pair<int *, ConnectionMonitor *>));
    args->first = socket;
    args->second = this;

    keepsMonitoringConnection(args);

    this->socketMapSemaphore->wait();
    this->socketCountdown.erase(*socket);
    this->socketMapSemaphore->post();
}

void * ConnectionMonitor::keepsMonitoringConnection(void *args) {
    std::pair<int *, ConnectionMonitor *> *pair = (std::pair<int *, ConnectionMonitor *> *) args;
    int socket = *(int *) pair->first;
    ConnectionMonitor *_this = (ConnectionMonitor *) pair->second;

    _this->socketMapSemaphore->wait();
    _this->socketCountdown[socket] = CONNECTION_LIVE;
    _this->socketMapSemaphore->post();

    bool connectionIsValid;
    do {
        sleep(CONNECTION_COUNTDOWN_STEP);
        _this->socketMapSemaphore->wait();
        _this->socketCountdown[socket] -= CONNECTION_COUNTDOWN_STEP;
        _this->socketMapSemaphore->post();
        connectionIsValid = _this->socketCountdown[socket] > 0;
    } while (connectionIsValid);
}

void ConnectionMonitor::refresh(int socket) {
    this->socketMapSemaphore->wait();
    this->socketCountdown[socket] = CONNECTION_LIVE;
    this->socketMapSemaphore->post();
}

void ConnectionMonitor::killSocket(int socket) {
    this->socketMapSemaphore->wait();
    this->socketCountdown.erase(socket);
    this->socketMapSemaphore->post();
}
