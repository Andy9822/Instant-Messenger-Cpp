#include <string>

using namespace std;
using std::string;

namespace servercommunicationmanager {

#define BUFFER_SIZE 100
#define WORD_MAX_LENGTH 256
#define MAX_NUMBER_OF_ACTIVE_USERS 100;

class ServerCommunicationManager {

    private:
        string stringBuffer [BUFFER_SIZE];
        string activeUsers[100]; // struct of user, group

    public:
        ServerCommunicationManager();
        ServerCommunicationManager(string user, string group);
        void listenForMessages(); // should be running in a separate thread
        string getMessageFromBuffer(string group);
        void sendMessageToGroup(string message, string gourp); //TODO: change this to use a instance of Message class
        void waitForNewUsers();
        void terminateUser(string user);

};
}  // namespace ServerCommunicationManager