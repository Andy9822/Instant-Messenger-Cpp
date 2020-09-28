#include "../../include/util/user.hpp"
#include <sstream>
#include <iostream>

namespace user{

User::User() {
    for (int i = 0; i < NUMBER_OF_SIMULTANEOUS_CONNECTIONS; i++) {
        this->sockets[i] = 0;
    }
}

User::User(string username) {
    User();
    this->username = username;
}

string User::getUsername() {
    return this->username;
}

void User::getActiveSockets(int* activeSocketsResult) {
    for (int i = 0; i < NUMBER_OF_SIMULTANEOUS_CONNECTIONS ; i++) {
        if (this->sockets[i] != 0) {
            activeSocketsResult[i] = this->sockets[i];
        }
    }
}

int User::registerSession(int socket) {
    for (int i = 0; i < NUMBER_OF_SIMULTANEOUS_CONNECTIONS; i++) {
        if (this->sockets[i] == 0){
            this->sockets[i] = socket;
            return 0;
        }
    }

    return -1;
}

void User::releaseSession(int socket) {
    for (int i = 0; i < NUMBER_OF_SIMULTANEOUS_CONNECTIONS; i++) {
        if (this->sockets[i] == socket){
            this->sockets[i] = 0;
            return;
        }
    }
}

void User::init_semaphore() {
    sem_init(&this->semaphore, 0, 1);
}

void User::wait_semaphore() {
    sem_wait(&this->semaphore);
}

void User::post_semaphore() {
    sem_post(&this->semaphore);
}

} // namespace user;