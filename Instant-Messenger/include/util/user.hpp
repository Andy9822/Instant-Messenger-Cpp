#include <string>
#include <ctime>
#include <list>
#include "exceptions.hpp"

using namespace std;
using std::string;

#define NUMBER_OF_SIMULTANEOUS_CONNECTIONS 2
namespace user {

class User {

    private:
        string username;
        // this list of sockets can only have two items
        int sockets[NUMBER_OF_SIMULTANEOUS_CONNECTIONS];
    
    public:
        User();
        User(string username);
        string getUsername();
        void getActiveSockets(int* activeSocketsResult);

        /*
        * Method to register a socket linked to a user
        * throws USER_SESSIONS_LIMIT_REACHED if the addition 
        * was not created due to limitation reached
        */
        void registerSession(int socket);
        void releaseSession(int socket);
        
};
}  //namespace user;