#include "../../include/util/user.hpp"
#include <sstream>
#include <iostream>

namespace user{


User::User(string username) : semaphore(1) {
    this->initSessionList();
    this->username = username;
}

void User::initSessionList() {
    this->semaphore.wait();
    this->sockets = new std::map<int, string>();
    this->semaphore.post();
}


string User::getUsername() {
    return this->username;
}

std::map<int, std::string> * User::getActiveSockets() {
    this->semaphore.wait();

    auto *activeSocketsResult = new map<int, string>();
    for (auto & socket : *sockets) {
        activeSocketsResult->insert(std::make_pair(socket.first, socket.second));
    }

    this->semaphore.post();

    return activeSocketsResult;
}

int User::registerSession(pair<const int, basic_string<char>> socket) {
    this->semaphore.wait();
    if(this->sockets->size() < 2){
        this->sockets->insert(socket);
        this->semaphore.post();
        return 0;
    }
    this->semaphore.post();
    return  -1;
}

/*void User::releaseSession(int socket) {
    this->semaphore.wait();
    for (int i = 0; i < NUMBER_OF_SIMULTANEOUS_CONNECTIONS; i++) {
        if (this->sockets[i] == socket){
            this->sockets[i] = 0;
            this->semaphore.post();
            return;
        }
    }
    this->semaphore.post();
}*/

} // namespace user;