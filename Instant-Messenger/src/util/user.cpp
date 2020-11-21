#include "../../include/util/user.hpp"
#include "../../include/util/definitions.hpp"
#include <iostream>
#include <algorithm>
#include <string.h>

namespace user {
    User::User(string username) : semaphore(1) {
        this->initSessionList();
        this->username = username;
    }

    void User::initSessionList() {
        this->semaphore.wait();
        this->clientIdentifiers = std::vector< pair <char *, int> >(); // pair (clientID, feSocket)
        this->semaphore.post();
    }

    string User::getUsername() {
        return this->username;
    }

    /**
     * Returns the identifiers for all sessions that this user has vector of pairs (clientID, feSocket)
     * @return
     */
    std::vector<pair<char *, int>> User::getActiveConnections() {
        return this->clientIdentifiers;
    }

    /**
     * The validation for the number of sockets is not the hard verification that we needed,
     * It is a week one because it is only checking in the group level. Server has a function to verify it in the upper layer
     * But it is good to hava some kind of validation here, as well
     * @param clientID
     * @return negative if error
     */
    int User::registerSession(char *clientID, int feSocket) {

        pair<char *, int> clientID_feSocket = pair<char *, int>();
        clientID_feSocket.first = (char*)malloc(UUID_SIZE*sizeof(char));
        strcpy(clientID_feSocket.first, clientID);
        clientID_feSocket.second = feSocket;

        this->semaphore.wait();
        if (this->clientIdentifiers.size() < MAX_NUMBER_OF_SIMULTANEOUS_CONNECTIONS ) {
            this->clientIdentifiers.push_back(clientID_feSocket);
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
     * @param feSocket
     * @param clientID
     */
    void User::releaseSession(char *clientID, int feSocket) {
        this->semaphore.wait();
        int deletion_index = 0;
        auto begin = this->clientIdentifiers.begin();
        for (auto userSession : this->clientIdentifiers) {
            if ( strcmp(userSession.first, clientID) == 0 && userSession.second == feSocket) {
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