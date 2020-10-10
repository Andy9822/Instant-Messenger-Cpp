#include "../../include/util/user.hpp"
#include <sstream>
#include <iostream>

namespace user {
    User::User(string username) : semaphore(1) {
        this->initSessionList();
        this->username = username;
    }

    void User::initSessionList() {
        this->semaphore.wait();
        this->sockets = std::vector<int>();
        this->semaphore.post();
    }


    string User::getUsername() {
        return this->username;
    }

    std::vector<int> User::getActiveSockets() {
        return sockets;
    }

    int User::registerSession(int socket) {
        this->semaphore.wait();
        this->sockets.push_back(socket);
        this->semaphore.post();
        return 0;
    }

    void User::printSockets() {
        for (auto socket : this->sockets) {
            cout << "printsockt::Socket: " << socket << endl;
        }
    }
} // namespace user;