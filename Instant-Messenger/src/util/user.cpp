#include "../../include/util/user.hpp"
#include <sstream>
#include <iostream>


namespace user{

User::User() {
    std::cout << "alo galera" << std::endl;
    std::cout << "semaforo endereco: " << &semaphore << std::endl;
    init_semaphore();
    wait_semaphore();
    for (int i = 0; i < NUMBER_OF_SIMULTANEOUS_CONNECTIONS; i++) {
        this->sockets[i] = 0;
    }
    post_semaphore();

    std::cout << "vou waitar" << std::endl;

    wait_semaphore();
    std::cout << "vou postar" << std::endl;
    post_semaphore();
    std::cout << "sla postou direito" << std::endl;

    std::cout << "this: " << this << std::endl;
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
    std::cout << "vou waitar tua mae" << std::endl;
    std::cout << "this: " << this << std::endl;
    wait_semaphore();
    std::cout << "mentira waitei nada" << std::endl;
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
    sem_init(&semaphore, 0, 1);
}

void User::wait_semaphore() {
    sem_wait(&semaphore);
}

void User::post_semaphore() {
    sem_post(&semaphore);
}

} // namespace user;