#include "../../include/util/user.hpp"
#include <sstream>


namespace user{

    User::User() {
        for (int i = 0; i < NUMBER_OF_SIMULTANEOUS_CONNECTIONS; i++) {
            this->sockets[i] = 0;
        }
    }

    User::User(string username) {
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

    void User::registerSession(int socket) {
        for (int i = 0; i < NUMBER_OF_SIMULTANEOUS_CONNECTIONS; i++) {
            if (this->sockets[i] == 0){
                this->sockets[i] = socket;
                return;
            }
        }
        throw USER_SESSIONS_LIMIT_REACHED;
    }

    void User::releaseSession(int socket) {
        for (int i = 0; i < NUMBER_OF_SIMULTANEOUS_CONNECTIONS; i++) {
            if (this->sockets[i] == socket){
                this->sockets[i] = 0;
                return;
            }
        }
    }
}