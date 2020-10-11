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

    /**
     * The validation for the number of sockets is not the hard verification that we needed,
     * It is a week on because it is only checking in the group level. Server has a function to verify it in the upper layer
     * But it is good to hava some kind of validation here, as well
     * @param socket
     * @return negative if error
     */
    int User::registerSession(int socket) {
        this->semaphore.wait();
        if ( sockets.size() < NUMBER_OF_SIMULTANEOUS_CONNECTIONS ) {
            this->sockets.push_back(socket);
            this->semaphore.post();
            return 0;
        } else {
            this->semaphore.post();
            return -1;
        }
    }

    void User::printSockets() {
        for (auto socket : this->sockets) {
            cout << "printsockt::Socket: " << socket << endl;
        }
    }
} // namespace user;