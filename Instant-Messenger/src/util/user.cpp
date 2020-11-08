#include "../../include/util/user.hpp"
#include <iostream>
#include <algorithm>

namespace user {
    User::User(string username) : semaphore(1) {
        this->initSessionList();
        this->username = username;
    }

    void User::initSessionList() {
        this->semaphore.wait();
        this->clientIdentifiers = std::vector< pair <int, int> >();
        this->semaphore.post();
    }


    string User::getUsername() {
        return this->username;
    }

    std::vector<pair<int, int> > User::getActiveConnections() {
        return this->clientIdentifiers;
    }

    /**
     * The validation for the number of sockets is not the hard verification that we needed,
     * It is a week on because it is only checking in the group level. Server has a function to verify it in the upper layer
     * But it is good to hava some kind of validation here, as well
     * @param clientIdentifier
     * @return negative if error
     */
    int User::registerSession(pair<int, int> clientIdentifier) {
        this->semaphore.wait();
        if (this->clientIdentifiers.size() < MAX_NUMBER_OF_SIMULTANEOUS_CONNECTIONS ) {
            this->clientIdentifiers.push_back(clientIdentifier);
            this->semaphore.post();
            return 0;
        } else {
            this->semaphore.post();
            return -1;
        }
    }

    void User::printSockets() {
        for (auto identification : this->clientIdentifiers) {
            cout << "printsockt::Client Dispositive Identifier: " << identification.first << " FE socket: " << identification.second << endl;
        }
    }

    void User::releaseSession(pair<int, int> clientIdentifier) {
        std::vector<pair <int, int> >::iterator element;
        this->semaphore.wait();
        element = find(this->clientIdentifiers.begin(), this->clientIdentifiers.end(), clientIdentifier); //TODO: check if it works
        if ( element != this->clientIdentifiers.end() ) {
            this->clientIdentifiers.erase(element);
        }
        this->semaphore.post();
    }
} // namespace user;