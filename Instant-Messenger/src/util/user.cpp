#include "../../include/util/user.hpp"
#include <sstream>
#include <iostream>
#include <thread>

namespace user{


User::User(string username) : semaphore(1) {
    cout << "Semaphore number (User constructor): " << &this->semaphore << endl;
    this->initSessionList();
    this->username = username;
}

void User::initSessionList() {
    this->semaphore.wait();
    for (int i = 0; i < NUMBER_OF_SIMULTANEOUS_CONNECTIONS; i++) {
        this->sockets[i] = 0;
    }
    this->semaphore.post();
}


string User::getUsername() {
    return this->username;
}

void User::getActiveSockets(int* activeSocketsResult) {
    this->semaphore.wait();
    for (int i = 0; i < NUMBER_OF_SIMULTANEOUS_CONNECTIONS ; i++) {
        if (this->sockets[i] != 0) {
            activeSocketsResult[i] = this->sockets[i];
        }
    }
    this->semaphore.post();
}

int User::registerSession(int socket) {
    cout << "Semaphore number (registerSession): " << &this->semaphore << endl;
    this->semaphore.wait();
    for (int i = 0; i < NUMBER_OF_SIMULTANEOUS_CONNECTIONS; i++) {
        if (this->sockets[i] == 0){
            this->sockets[i] = socket;
            this->semaphore.post();
            return 0;
        }
    }
    this->semaphore.post();
    return  -1;
}

void User::releaseSession(int socket) {
    this->semaphore.wait();
    for (int i = 0; i < NUMBER_OF_SIMULTANEOUS_CONNECTIONS; i++) {
        if (this->sockets[i] == socket){
            this->sockets[i] = 0;
            this->semaphore.post();
            return;
        }
    }
    this->semaphore.post();
}

} // namespace user;