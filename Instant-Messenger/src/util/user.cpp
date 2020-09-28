#include "../../include/util/user.hpp"
#include <sstream>
#include <iostream>

sem_t semaphore;

namespace user{

User::User() {
    init_semaphore();
    wait_semaphore();
    for (int i = 0; i < NUMBER_OF_SIMULTANEOUS_CONNECTIONS; i++) {
        this->sockets[i] = 0;
    }
    post_semaphore();
}

User::User(string username) {
    User();
    this->username = username;
}

string User::getUsername() {
    return this->username;
}

void User::getActiveSockets(int* activeSocketsResult) {
    wait_semaphore();
    for (int i = 0; i < NUMBER_OF_SIMULTANEOUS_CONNECTIONS ; i++) {
        if (this->sockets[i] != 0) {
            activeSocketsResult[i] = this->sockets[i];
        }
    }
    post_semaphore();
}

int User::registerSession(int socket) {
    wait_semaphore();
    for (int i = 0; i < NUMBER_OF_SIMULTANEOUS_CONNECTIONS; i++) {
        if (this->sockets[i] == 0){
            this->sockets[i] = socket;
            post_semaphore();
            return 0;
        }
    }
    post_semaphore();

    return -1;
}

void User::releaseSession(int socket) {
    wait_semaphore();
    for (int i = 0; i < NUMBER_OF_SIMULTANEOUS_CONNECTIONS; i++) {
        if (this->sockets[i] == socket){
            this->sockets[i] = 0;
            post_semaphore();
            return;
        }
    }
    post_semaphore();
}

void User::init_semaphore() {
    sem_init(/*&this->*/&semaphore, 0, 1);
}

void User::wait_semaphore() {
    sem_wait(/*&this->*/&semaphore);
}

void User::post_semaphore() {
    sem_post(/*&this->*/&semaphore);
}

} // namespace user;