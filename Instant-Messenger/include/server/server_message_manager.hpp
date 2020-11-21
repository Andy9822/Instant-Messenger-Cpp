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
        void broadcastMessageToUsers(Message message, vector< pair <char *, int> > connectionIds);
        void sendMessageToSocketId(Message message, char *clientID, int feSocket);
        void sendPacketToSocketId(Packet* packet, int socket);

        void sendMessageToAddress(Message message, string clientId, string feAddress);

    private:
        FeAddressBook feAddressBook;

        void sendMessageToSocketId(Message message, string clientID, string feAddress);

        void sendMessageToSocketId(Message message, string clientID, int feSocket);
    };
}

#endif