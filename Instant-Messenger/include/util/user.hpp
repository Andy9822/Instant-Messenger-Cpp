#ifndef USER_HPP
#define USER_HPP

#include <string>
#include <ctime>
#include <list>
#include "exceptions.hpp"
#include "Semaphore.hpp"

using namespace std;
using std::string;

#define NUMBER_OF_SIMULTANEOUS_CONNECTIONS 2

namespace user {

class User {

    private:
        string username;
        // this list of sockets can only have two items
        //TODO: CONVERT THIS TO A LIST OF OBJ OR MAP CONTAINING (GROUP, SOCKET_ID)
        int sockets[NUMBER_OF_SIMULTANEOUS_CONNECTIONS];
        Semaphore semaphore;
    
    public:
        User(string username);
        string getUsername();
        void getActiveSockets(int* activeSocketsResult);

        /*
        * Method to register a socket linked to a user
        * throws USER_SESSIONS_LIMIT_REACHED if the addition 
        * was not created due to limitation reached
        */
        int registerSession(int socket);
        void releaseSession(int socket);
        void initSessionList();

        
};
}  //namespace user;

#endif