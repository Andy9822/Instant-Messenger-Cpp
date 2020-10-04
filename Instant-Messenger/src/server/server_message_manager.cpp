#include "../../include/server/server_message_manager.hpp"


namespace servermessagemanager {

    void ServerMessageManager::broadcastMessageToUsers(Message message, std::list<User *> users)
    {
        //todo: verify how to only call sockets for correct group
        //todo: how to display message for the one who sent it
        Packet* sendingPacket = new Packet((char*)message.getUser().c_str(), (char*)message.getGroup().c_str(), (char*)message.getText().c_str());
        cout << "Packet: " << sendingPacket ->username << "  " << sendingPacket -> group << "  " << sendingPacket -> message << " TO--> \n";

        for (auto userItr = users.begin(); userItr != users.end(); userItr++) {
            cout << "user: " << (*userItr)->getUsername() << "  WITH-->-\n";
            int* activeSockets = new int[NUMBER_OF_SIMULTANEOUS_CONNECTIONS];
            (*userItr)->getActiveSockets(activeSockets);
            cout << "socket: " << activeSockets[0] << "  \n";
            cout << "socket: " << activeSockets[1] << "  \n";
            for (int i = 0; i < NUMBER_OF_SIMULTANEOUS_CONNECTIONS ; i++) {
                if (activeSockets[i] != 0) {
                    sendPacket(activeSockets[i], sendingPacket);
                }
            }
        }
    }
}

