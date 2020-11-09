#include "../../include/server/server_message_manager.hpp"


namespace servermessagemanager {

    /**
     * This method is responsiblefor building a package from a message and send it thought the socket :)
     * @param message
     * @param clientIdentifier
     */
    void ServerMessageManager::sendMessageToSocketId(Message message, pair<int, int> clientIdentifier)
    {
        int clientDispositiveIdentifier = clientIdentifier.first;
        int frontEndSocket = clientIdentifier.second;

        Packet* sendingPacket = new Packet((char*)message.getUser().c_str(), (char*)message.getGroup().c_str(), (char*)message.getText().c_str(), clientDispositiveIdentifier, message.getTime());
        sendPacket(frontEndSocket, sendingPacket);
    }
    
    /**
     * This method is responsiblefor for sending a Packet to a socket :)
     * @param packet
     * @param clientIdentifier
     */
    void ServerMessageManager::sendPacketToSocketId(Packet* packet, int socket)
    {
        sendPacket(socket, packet);
    }

    /**
     * This bro can be used to send the messages to a list of sockets.
     * Hint: it is very useful in the group class for consuming the message queue
     * @param message
     * @param connectionIds
     */
    void ServerMessageManager::broadcastMessageToUsers(Message message, vector< pair<int, int> > connectionIds)
    {
        for ( auto clientConnection : connectionIds) {
            sendMessageToSocketId(message, clientConnection);
        }
    }
}

