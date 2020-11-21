#include "../../include/server/server_message_manager.hpp"


namespace servermessagemanager {

    ServerMessageManager::ServerMessageManager(FeAddressBook feAddressBook) {
        this->feAddressBook = feAddressBook;
    }

    /**
     * This method is responsible for building a package from a message and send it thought the socket :)
     * @param message
     * @param clientID
     */
    void ServerMessageManager::sendMessageToSocketId(Message message, string clientID, int feSocket) //TODO: update this to string,string
    {
        Packet* sendingPacket = new Packet(
                (char*)message.getUser().c_str(),
                (char*)message.getGroup().c_str(),
                (char*)message.getText().c_str(),
                message.getTime(),
                (char*)clientID.c_str(), // this field is the client's identifier in the FE. aka user_id for some
               MESSAGE_PACKET);

        sendPacket(feSocket, sendingPacket); //TODO: update this to string,string
    }

    /**
     * This method is responsible for building a package from a message and send it thought the socket :)
     * @param message
     * @param clientID
     */
    void ServerMessageManager::sendMessageToSocketId(Message message, string clientID, string feAddress) //TODO: update this to string,string
    {

        int socketId = this->feAddressBook.getInternalSocketId(feAddress);

        if (socketId == 0) {
            cout << "[ERROR] the address provided for the FE does not have any socket" << endl;
        }


        Packet* sendingPacket = new Packet(
                (char*)message.getUser().c_str(),
                (char*)message.getGroup().c_str(),
                (char*)message.getText().c_str(),
                message.getTime(),
                (char*)clientID.c_str(), // this field is the client's identifier in the FE. aka user_id for some
                MESSAGE_PACKET);

        sendPacket(socketId, sendingPacket); //TODO: update this to string,string
    }

    /**
     * Makes the search for socket id in the feAddressBook and sends the message to the right socket
     * @param message
     * @param clientId
     * @param feAddress
     */
    void ServerMessageManager::sendMessageToAddress(Message message, string clientId, string feAddress) {

        int socketId = this->feAddressBook.getInternalSocketId(feAddress);

        if (socketId == 0) {
            cout << "[ERROR] the address provided for the FE does not have any socket" << endl;
        }

        sendMessageToSocketId(message, clientId, socketId)
    }


    /**
     * This method is responsiblefor for sending a Packet to a socket :)
     * @param packet
     * @param clientIdentifier
     */
    void ServerMessageManager::sendPacketToSocketId(Packet* packet, int socket)
    {
        sendPacket(socket, packet); //TODO: update this to string,string
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
    void ServerMessageManager::broadcastMessageToUsers(Message message, vector< pair<char *, int> > connectionIds) //TODO: update this to string,string
    {
        for ( auto clientConnection : connectionIds) {
            sendMessageToSocketId(message, clientConnection.first, clientConnection.second); //TODO: update this to string,string
        }
    }
}

