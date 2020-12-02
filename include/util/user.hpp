#ifndef USER_HPP
#define USER_HPP

#include <string>
#include <ctime>
#include <list>
#include <map>
#include <vector>
#include "exceptions.hpp"
#include "Semaphore.hpp"

using namespace std;
using std::string;

#define MAX_NUMBER_OF_SIMULTANEOUS_CONNECTIONS 2

namespace user {

class User {

    private:
        string username;
        std::vector<pair <string, string> > clientIdentifiers; // clientID, feSocket
        Semaphore semaphore;
    
    public:
        User(string username);
        string getUsername();
        std::vector<pair<string, string>> getActiveConnections();

        /*
        * Method to register a socket linked to a user
        * throws USER_SESSIONS_LIMIT_REACHED if the addition
        * was not created due to limitation reached
        */
        int registerSession(string clientID, string feAddress);
        void releaseSession(string clientID, string feAddress);
        void initSessionList();

        bool operator == (const User& s) const { return username == s.username; }
        bool operator != (const User& s) const { return !operator==(s); }

        void printSockets();
        
};
}  //namespace user;

#endif