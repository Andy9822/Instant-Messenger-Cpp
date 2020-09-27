#include <string>
#include "include/util/message.hpp"

using namespace std;
using std::string;
using namespace message;
namespace servermessage {

class ServerMessageManager {

    public:
        ServerMessageManager();
 
        /*
        * This method is responsible for sending the messages to the
        * communication layer
        */
        void sendMessage(Message message, Users users);
 
};
}  //namespace clientmessagemanager