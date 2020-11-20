#include "../../include/server/server_message_manager.hpp"


namespace servermessagemanager {

    /**
     * This method is responsible for building a package from a message and send it thought the socket :)
     * @param message
     * @param clientID
     */
    void ServerMessageManager::sendMessageToSocketId(Message message, char *clientID, int feSocket)
    {
        Packet* sendingPacket = new Packet(
                (char*)message.getUser().c_str(),
                (char*)message.getGroup().c_str(),
                (char*)message.getText().c_str(),
                message.getTime(),
                clientID, // this field is the client's identifier in the FE. aka user_id for some
               MESSAGE_PACKET);

        sendPacket(feSocket, sendingPacket);
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
     *
     * the pair connectionIds contains: (clientID, feSocket)
     *
     * @param message
     * @param connectionIds
     */
    void ServerMessageManager::broadcastMessageToUsers(Message message, vector< pair<char *, int> > connectionIds)
    {
        for ( auto clientConnection : connectionIds) {
            sendMessageToSocketId(message, clientConnection.first, clientConnection.second);
        }
    }
}

