#ifndef SERVER_MESSAGE_HPP
#define SERVER_MESSAGE_HPP

#include "../util/Packet.hpp"
#include "../util/message.hpp"
#include "../util/user.hpp"
#include "../util/Socket.hpp"
#include "FeAddressBook.hpp"

using namespace message;
using namespace user;

namespace servermessagemanager {

    class ServerMessageManager : public Socket{
    public:
        ServerMessageManager(FeAddressBook feAddressBook);
        void broadcastMessageToUsers(Message message, vector< pair <string, string> > connectionIds);
        void sendMessageToSession(Message message, string clientID, string feAddress);
        void sendPacketToSocketId(Packet* packet, string feAddress);
        void sendMessageToAddress(Message message, string clientId, string feAddress);
    private:
        FeAddressBook feAddressBook;
        void sendMessageToSocketId(Message message, string clientID, int feSocket); //An aux to send the message when we already converted the address to the actual socket id
        int getSocketFromAddress(const string &feAddress);
    };
}

#endif