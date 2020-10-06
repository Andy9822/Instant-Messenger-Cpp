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
        this->sockets = new std::map<int, string>();
        this->semaphore.post();
    }


    string User::getUsername() {
        return this->username;
    }

    std::map<int, std::string> *User::getActiveSockets() {
        return sockets;
    }

    int User::registerSession(pair<const int, basic_string<char>> socket) {
        this->semaphore.wait();
        if (this->sockets->size() < 2) {
            this->sockets->insert(socket);
            this->semaphore.post();
            return 0;
        }
        this->semaphore.post();
        return -1;
    }

    void User::printSockets() {
        for (auto const &socket : *sockets) {
            cout << "printsockt::Socket: " << socket.first << " from group: " << socket.second << endl;
        }
    }
} // namespace user;