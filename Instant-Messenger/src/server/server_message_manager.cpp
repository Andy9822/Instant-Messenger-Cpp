#include "../../include/server/server_message_manager.hpp"


namespace servermessagemanager {

    void ServerMessageManager::broadcastMessageToUsers(Message message, std::list<User *> users)
    {
        //todo: verify how to only call sockets for correct group,
        // as we don't have information between groups and sockets at this moment

        Packet* sendingPacket = new Packet((char*)message.getUser().c_str(), (char*)message.getGroup().c_str(), (char*)message.getText().c_str(), message.getTime());
        //cout << "Packet: " << sendingPacket ->username << "  " << sendingPacket -> group << "  " << sendingPacket -> message << " TO--> \n";

        for (auto userItr = users.begin(); userItr != users.end(); userItr++) {
            //cout << "user: " << (*userItr)->getUsername() << "  WITH-->-\n";
            int* activeSockets = new int[NUMBER_OF_SIMULTANEOUS_CONNECTIONS];
            (*userItr)->getActiveSockets(activeSockets);
            //cout << "socket: " << activeSockets[0] << "  \n";
            //cout << "socket: " << activeSockets[1] << "  \n";
            for (int i = 0; i < NUMBER_OF_SIMULTANEOUS_CONNECTIONS ; i++) {
                //TODO: VERIFY IF SOCKET IS FROM SAME GROUP
                if (activeSockets[i] != 0 /*activeSockets.value == message.getGroup()*/) {
                    sendPacket(activeSockets[i], sendingPacket);
                }
            }
        }
    }
}

