#include <string>
#include <ctime>
#include <list>
#include "exceptions.hpp"
#include <semaphore.h>

using namespace std;
using std::string;

#define NUMBER_OF_SIMULTANEOUS_CONNECTIONS 2
namespace user {

class User {

    private:
        sem_t semaphore;
        string username;
        // this list of sockets can only have two items
        int sockets[NUMBER_OF_SIMULTANEOUS_CONNECTIONS];
        void init_semaphore();
        void wait_semaphore();
        void post_semaphore();
    
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