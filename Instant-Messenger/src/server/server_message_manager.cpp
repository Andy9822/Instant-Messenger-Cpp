#include "../../include/server/server_message_manager.hpp"


namespace servermessagemanager {

    void ServerMessageManager::sendMessageToSocketId(Message message, int socketId)
    {
        Packet* sendingPacket = new Packet((char*)message.getUser().c_str(), (char*)message.getGroup().c_str(), (char*)message.getText().c_str(), message.getTime());
        sendPacket(socketId, sendingPacket);
    }

    void ServerMessageManager::broadcastMessageToUsers(Message message, std::list<User *> users)
    {
        Packet* sendingPacket = new Packet((char*)message.getUser().c_str(), (char*)message.getGroup().c_str(), (char*)message.getText().c_str(), message.getTime());

        for (auto const& user : users) {
            std::map<int, std::string> *activeSockets = user->getActiveSockets();

            for (auto const& socket : *activeSockets) {
                if(socket.second == message.getGroup())
                    sendPacket(socket.first, sendingPacket);
            }
        }
    }
}

