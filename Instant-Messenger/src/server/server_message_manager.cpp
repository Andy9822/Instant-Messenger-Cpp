#include "../../include/server/server_message_manager.hpp"


namespace servermessagemanager {

    /**
     * This method is responsiblefor building a package from a message and send it thought the socket :)
     * @param message
     * @param socketId
     */
    void ServerMessageManager::sendMessageToSocketId(Message message, int socketId)
    {
        Packet* sendingPacket = new Packet((char*)message.getUser().c_str(), (char*)message.getGroup().c_str(), (char*)message.getText().c_str(), message.getTime());
        sendPacket(socketId, sendingPacket);
    }

    /**
     * This bro can be used to send the messages to a list of sockets.
     * Hint: it is very useful in the group class for consuming the message queue
     * @param message
     * @param sockets
     */
    void ServerMessageManager::broadcastMessageToUsers(Message message, vector<int> sockets)
    {
        for ( auto socket : sockets) {
            sendMessageToSocketId(message, socket);
        }
    }
}

