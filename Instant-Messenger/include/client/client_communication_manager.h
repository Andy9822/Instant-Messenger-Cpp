#include <string>

using namespace std;
using std::string;

namespace clientcommunicationmanager {

#define BUFFER_SIZE 10
#define WORD_MAX_LENGTH 256

class ClientCommunicationManager {

    std::string stringBuffer [BUFFER_SIZE];

    public:
        ClientCommunicationManager();
        ClientCommunicationManager(string user, string group);

        void listenForMessages(); // should be running in a separate thread
        string getMessageFromBuffer();
        void sendMessageToServer(string message);

};
}  // namespace clientcommunicationmanager