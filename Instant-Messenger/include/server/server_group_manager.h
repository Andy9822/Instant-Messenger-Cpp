#include <string>
#include "include/util/message.hpp"
#include "include/util/user.hpp"

using namespace std;
using std::string;
using namespace message;
using namespace user;
namespace servermessage {

class ServerMessageManager {

    public:
        ServerMessageManager();
 
        /*
        * This method is responsible for sending the messages to the
        * communication layer
        */
        void sendMessage(Message message, User* users);
 
};
}  //namespace clientmessagemanager