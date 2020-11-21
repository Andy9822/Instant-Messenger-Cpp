#include "../../include/util/user.hpp"
#include "../../include/util/definitions.hpp"
#include <iostream>
#include <algorithm>

namespace user {
    User::User(string username) : semaphore(1) {
        this->initSessionList();
        this->username = username;
    }

    void User::initSessionList() {
        this->semaphore.wait();
        this->clientIdentifiers = std::vector< pair <string, string> >(); // pair (clientID, feSocket)
        this->semaphore.post();
    }

    string User::getUsername() {
        return this->username;
    }

    /**
     * Returns the identifiers for all sessions that this user has vector of pairs (clientID, feSocket)
     * @return
     */
    std::vector<pair<string, string>> User::getActiveConnections() {
        return this->clientIdentifiers;
    }

    /**
     * The validation for the number of sockets is not the hard verification that we needed,
     * It is a week one because it is only checking in the group level. Server has a function to verify it in the upper layer
     * But it is good to hava some kind of validation here, as well
     * @param clientID
     * @return negative if error
     */
    int User::registerSession(string clientID, string feAddress) { // TODO: update to use string,string

        pair<string, string> clientID_feSocket = pair<string, string>();
        clientID_feSocket.first.assign(clientID); //TODO: debug to confirm
        clientID_feSocket.second.assign(feAddress);

        this->semaphore.wait();
        if (this->clientIdentifiers.size() < MAX_NUMBER_OF_SIMULTANEOUS_CONNECTIONS ) {
            this->clientIdentifiers.push_back(clientID_feSocket); // TODO: update this shit
            this->semaphore.post();
            return 0;
        } else {
            this->semaphore.post();
            return -1;
        }
    }

    /**
     * Releases a connection for a user
     * Since a user can be connected in different FEs and with different dispositives in the same FE, the
     * client's identifier is maintained by a pair (clientID, feSocket). We need to use this pair
     * to delete the existing session accordingly
     *
     * @param clientID
     * @param feAddress
     */
    void User::releaseSession(string clientID, string feAddress) {
        this->semaphore.wait();
        int deletion_index = 0;
        auto begin = this->clientIdentifiers.begin();
        for (auto userSession : this->clientIdentifiers) {
            if ( userSession.first.compare(clientID) == 0 && userSession.first.compare(feAddress) ) {
                this->clientIdentifiers.erase(begin + deletion_index); // will delete the deletion_index's item
            }
            deletion_index += 1;
        }

        this->printSockets();

        this->semaphore.post();
    }

    void User::printSockets() {
        for (auto identification : this->clientIdentifiers) {
            cout << "printsockt::Client Dispositive Identifier: " << identification.first << " FE socket: " << identification.second << endl;
        }
    }
} // namespace user;