#ifndef PACKET_HPP
#define PACKET_HPP

#include <string.h> 
#include "definitions.hpp"
#include <ctime>

using namespace std;

struct Packet 
{
    int status;
    char username[USERNAME_MAX_SIZE];
    char group[GROUP_MAX_SIZE];
    char message[MESSAGE_MAX_SIZE];
    int clientSocket;
    int frontEndSocket;
    int clientDispositiveIdentifier;
    time_t timestamp;
    int type;

    Packet() {
        this->type = MESSAGE_PACKET;
    }

    Packet(int type) {
        this->type = type;
    }

    Packet(char* username, char* group, char* message, int clientDispositiveIdentifier, time_t timestamp) {
      this->status = 1440;
      strncpy(this->username, username, USERNAME_MAX_SIZE - 1);
      strncpy(this->group, group, GROUP_MAX_SIZE - 1);
      strncpy(this->message, message, MESSAGE_MAX_SIZE - 1);
      this->clientDispositiveIdentifier = clientDispositiveIdentifier;
      this->timestamp = timestamp;
    }
    bool isKeepAlive() {
        return this->type == KEEP_ALIVE_PACKET;
    }
    bool isMessage() {
        return this->type == MESSAGE_PACKET;
    }
    bool isElection() {
        return this->type == ELECTION_PACKET;
    }

    bool isJoinMessage() {
        return this->type == JOIN_PACKET;
    }

    bool isDisconnect() {
        return this->type == DISCONNECT_PACKET;
    }

    bool isAckPacket() {
        return this->type == ACK_PACKET;
    }
};

#endif
